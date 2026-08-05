"""F Prime cFS GroundSystem runner

This script is designed to replace fprime-gds with the cFS GroundSystem GDS. It generates
cFS GroundSystem telemetry and command configuration from the F Prime dictionary, starts
the TM/TC frame bridge to the fprime_gds GdsBridge application, and runs the
cFS GroundSystem GUI against the generated configuration.
"""

import atexit
import os
import shutil
import sys
import tempfile

from pathlib import Path
from typing import Any, Dict, Tuple

from fprime_gds.executables.cli import (
    BinaryDeployment,
    ConfigDrivenParser,
    DictionaryParser,
    LogDeployParser,
    ParserBase,
    PluginArgumentParser,
)
from fprime_gds.executables.run_deployment import launch_app, launch_process

from . import comm
from .dictionary import Dictionary
from .generate import generate


class CfsGroundSystemParser(ParserBase):
    """Parser for cFS GroundSystem specific arguments"""

    DESCRIPTION = "cFS GroundSystem settings for use with F Prime"

    def get_arguments(self) -> Dict[Tuple[str, ...], Dict[str, Any]]:
        """Arguments to handle the cFS GroundSystem"""
        arguments = {
            ("--ground-system-dir",): {
                "action": "store",
                "default": None,
                "type": Path,
                "required": True,
                "help": "Path to the cFS-GroundSystem directory (e.g. tools/cFS-GroundSystem)",
            },
            ("--no-copy",): {
                "action": "store_true",
                "default": False,
                "help": "Write configuration into the cFS-GroundSystem directory in place "
                        "instead of a temporary copy",
            },
        }
        arguments.update(comm.BRIDGE_ARGUMENTS)
        return arguments

    def handle_arguments(self, args, **kwargs):
        """Handle arguments as parsed"""
        if not (args.ground_system_dir / "GroundSystem.py").is_file():
            raise Exception(
                f"[ERROR] {args.ground_system_dir} does not contain GroundSystem.py"
            )
        return args


def construct_ground_system(parsed_args, dictionary: Dictionary) -> Path:
    """Construct the cFS GroundSystem directory holding the generated configuration

    Unless --no-copy was supplied, the cFS-GroundSystem checkout is copied to a temporary
    directory (destroyed on exit) so the checkout is left pristine.
    """
    ground_system_dir = parsed_args.ground_system_dir
    if not parsed_args.no_copy:
        working_dir = Path(tempfile.mkdtemp()) / "cFS-GroundSystem"
        atexit.register(lambda: shutil.rmtree(working_dir.parent, ignore_errors=True))
        shutil.copytree(ground_system_dir, working_dir)
        ground_system_dir = working_dir
    print(f"[INFO] Generating cFS GroundSystem configuration in {ground_system_dir}")
    generate(dictionary, ground_system_dir)
    return ground_system_dir


def launch_ground_system(ground_system_dir: Path):
    """Launch the cFS GroundSystem GUI"""
    return launch_process(
        [sys.executable, str(ground_system_dir / "GroundSystem.py")],
        env=os.environ.copy(),
    )


def parse_args():
    """Parse the arguments for the F Prime cFS GroundSystem runner"""
    argument_handlers = [
        DictionaryParser,
        BinaryDeployment,
        LogDeployParser,
        PluginArgumentParser,
        CfsGroundSystemParser,
    ]
    if "FPRIME_GDS_CONFIG_PATH" in os.environ:
        ConfigDrivenParser.set_default_configuration(
            Path(os.environ["FPRIME_GDS_CONFIG_PATH"])
        )
    args, _ = ConfigDrivenParser.parse_args(
        argument_handlers, "Run F Prime deployment and the cFS GroundSystem GDS"
    )
    return args


def main():
    """Main entrypoint for the F Prime cFS GroundSystem runner"""
    parsed_args = parse_args()
    try:
        dictionary = Dictionary(parsed_args.dictionary)
        ground_system_dir = construct_ground_system(parsed_args, dictionary)

        bridge = comm.bridge_from_arguments(parsed_args, dictionary)
        atexit.register(bridge.stop)
        bridge.start()

        launched_apps = [launch_app] if parsed_args.app is not None else []
        processes = [launcher(parsed_args) for launcher in launched_apps]
        processes.append(launch_ground_system(ground_system_dir))
        print("[INFO] F Prime cFS GroundSystem is now running. CTRL-C to shutdown all components.")
        processes[-1].wait()
    except KeyboardInterrupt:
        print("[INFO] CTRL-C received. Exiting.")
    except Exception as exc:
        print(f"[ERROR] Shutting down F Prime cFS GroundSystem due to error: {exc}", file=sys.stderr)
        return 1
    # Processes are killed atexit
    return 0


if __name__ == "__main__":
    sys.exit(main())
