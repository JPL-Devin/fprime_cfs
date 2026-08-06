"""fprime_cfs.generate: cFS GroundSystem configuration generation entry point

Reads an F Prime JSON dictionary and writes cFS GroundSystem telemetry and command
configuration into a cFS-GroundSystem checkout (or a mirror of its Subsystems layout).
"""

import argparse
import sys
from pathlib import Path

from .commands import TO_LAB_DEFAULT_MESSAGE_ID, generate_commands
from .dictionary import Dictionary, DictionaryError
from .telemetry import generate_telemetry


def generate(
    dictionary: Dictionary,
    ground_system: Path,
    command_host: str = "127.0.0.1",
    command_port: int = 1234,
    to_lab_message_id: int = TO_LAB_DEFAULT_MESSAGE_ID,
):
    """Generate all cFS GroundSystem configuration for the supplied dictionary

    Args:
        dictionary: parsed F Prime dictionary
        ground_system: path to a cFS-GroundSystem directory (containing Subsystems/)
        command_host: address the command GUI sends command datagrams to
        command_port: port the command GUI sends command datagrams to
        to_lab_message_id: message id of the TO_LAB command page and quick button
    """
    ground_system = Path(ground_system)
    tlm_gui_dir = ground_system / "Subsystems" / "tlmGUI"
    cmd_gui_dir = ground_system / "Subsystems" / "cmdGui"
    for directory in (tlm_gui_dir, cmd_gui_dir):
        if not directory.is_dir():
            raise DictionaryError(
                f"'{directory}' is not a directory; expected a cFS-GroundSystem checkout"
            )
    stream_id = generate_telemetry(dictionary, tlm_gui_dir)
    message_id, skipped = generate_commands(
        dictionary,
        cmd_gui_dir,
        command_host=command_host,
        command_port=command_port,
        to_lab_message_id=to_lab_message_id,
    )
    print(f"[INFO] Telemetry page generated for stream id {stream_id:#06x}")
    print(f"[INFO] Command page generated for message id {message_id:#06x}")
    for skip in skipped:
        print(
            f"[WARNING] Command skipped (no MiniCmdUtil representation): {skip}",
            file=sys.stderr,
        )


def main():
    """Entry point for fprime-cfs-config"""
    parser = argparse.ArgumentParser(
        description="Generate cFS GroundSystem configuration from an F Prime dictionary"
    )
    parser.add_argument(
        "--dictionary",
        required=True,
        type=Path,
        help="Path to the F Prime JSON dictionary",
    )
    parser.add_argument(
        "--ground-system",
        required=True,
        type=Path,
        help="Path to the cFS-GroundSystem directory to write configuration into",
    )
    parser.add_argument(
        "--command-address",
        default="127.0.0.1",
        help="Command UDP address written to the command page. Default: %(default)s",
    )
    parser.add_argument(
        "--command-port",
        type=int,
        default=1234,
        help="Command UDP port written to the command page. Default: %(default)s",
    )
    parser.add_argument(
        "--to-lab-message-id",
        type=lambda value: int(value, 0),
        default=TO_LAB_DEFAULT_MESSAGE_ID,
        help="Message id of the TO_LAB command application. Default: 0x1880",
    )
    args = parser.parse_args()
    try:
        generate(
            Dictionary(args.dictionary),
            args.ground_system,
            command_host=args.command_address,
            command_port=args.command_port,
            to_lab_message_id=args.to_lab_message_id,
        )
    except DictionaryError as error:
        print(f"[ERROR] {error}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
