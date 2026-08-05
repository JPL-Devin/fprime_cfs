"""fprime_cfs_gds.commands: cFS GroundSystem command configuration generation

Generates the cmdGui configuration (command-pages.txt, quick-buttons.txt, the command
definition pickle, and per-command parameter pickles) for the F Prime command dictionary.

F Prime commands travel to the flight software as cFS command packets on the uplink
message id (command type bit and secondary header flag set over the F Prime command
APID). All commands share the single cFS command function code published in the
dictionary as ComCfg.FprimeCommandFunctionCode; the flight-side router strips the cFS
command secondary header and hands the payload to the F Prime command dispatcher.

The payload is a standard F Prime command packet: the FW_PACKET_COMMAND framing
descriptor, the command opcode, then the command arguments, all big-endian. The framing
descriptor and opcode are prepended as the first two parameters of every generated
command; their required values are fixed and included in each parameter description.
"""

import pickle
import re
from pathlib import Path
from typing import List, Tuple

from .dictionary import Command, Dictionary, DictionaryError, ResolvedType

COMMAND_PAGES_FILE = "command-pages.txt"
QUICK_BUTTONS_FILE = "quick-buttons.txt"
COMMAND_DEFINITION_FILE = "fprime_cmds"

# Space packet stream identifier bits (CCSDS primary header, big-endian 16 bits)
STREAM_ID_TYPE_COMMAND = 0x1000
STREAM_ID_SEC_HDR = 0x0800

# MiniCmdUtil type flags by (size, signed); page endianness (BE) supplies the byte order
_CMD_UTIL_FLAGS = {
    (1, False): "--uint8",
    (1, True): "--int8",
    (2, False): "--uint16",
    (2, True): "--int16",
    (4, False): "--uint32",
    (4, True): "--int32",
    (8, False): "--uint64",
    (8, True): "--int64",
}

# Fw serializable boolean representation
BOOL_TRUE = 0xFF
BOOL_FALSE = 0x00


class _Parameter:
    """One cmdGui parameter entry"""

    def __init__(self, name: str, resolved: ResolvedType, description: str):
        flag = _CMD_UTIL_FLAGS.get((resolved.size, resolved.signed))
        if resolved.kind == "float":
            # MiniCmdUtil has no float support: floats are entered as raw IEEE-754 bits
            flag = _CMD_UTIL_FLAGS[(resolved.size, False)]
            description += f" [{resolved.name}: enter the raw IEEE-754 bit pattern as an unsigned integer]"
        elif resolved.kind == "bool":
            flag = _CMD_UTIL_FLAGS[(1, False)]
            description += (
                f" [bool: enter {BOOL_FALSE} for FALSE, {BOOL_TRUE} for TRUE]"
            )
        elif resolved.kind == "enum":
            values = ", ".join(
                f"{name}={value}"
                for name, value in (resolved.enum_constants or {}).items()
            )
            description += f" [{resolved.name}: {values}]"
        elif resolved.kind == "integer":
            description += f" [{resolved.name}]"
        if flag is None:
            raise DictionaryError(f"No cmdUtil flag for type '{resolved.name}'")
        self.name = name
        self.data_type = flag.strip("-")
        self.flag = flag
        self.description = description.replace(",", ";").replace("\n", " ")


def _sanitize(name: str) -> str:
    """Sanitize a command name into a file name"""
    return re.sub(r"[^A-Za-z0-9_]", "_", name)


def _command_parameters(
    dictionary: Dictionary,
    command: Command,
    descriptor: ResolvedType,
    opcode_type: ResolvedType,
) -> List[_Parameter]:
    """Build the parameter list for a command: descriptor, opcode, then arguments"""
    parameters = [
        _Parameter(
            "FramingDescriptor",
            descriptor,
            "F Prime framing descriptor. Enter 0 (FW_PACKET_COMMAND)",
        ),
        _Parameter(
            "Opcode",
            opcode_type,
            f"F Prime opcode of {command.name}. Enter {command.opcode} ({command.opcode:#x})",
        ),
    ]
    for param in command.params:
        resolved = dictionary.resolve_type(param["type"])
        parameters.append(
            _Parameter(
                param["name"], resolved, param.get("annotation") or param["name"]
            )
        )
    return parameters


def generate_commands(
    dictionary: Dictionary,
    cmd_gui_dir: Path,
    command_host: str = "127.0.0.1",
    command_port: int = 1234,
) -> Tuple[int, List[str]]:
    """Generate cmdGui configuration for the F Prime command dictionary

    Returns the command message id and the list of commands skipped because their
    arguments have no MiniCmdUtil representation (e.g. strings).
    """
    function_code = dictionary.get_constant("ComCfg.FprimeCommandFunctionCode")
    descriptor = dictionary.resolve_definition(
        dictionary.type_definitions["FwPacketDescriptorType"]
    )
    opcode_type = dictionary.resolve_definition(
        dictionary.type_definitions["FwOpcodeType"]
    )
    message_id = (
        dictionary.get_apid("FW_PACKET_COMMAND")
        | STREAM_ID_TYPE_COMMAND
        | STREAM_ID_SEC_HDR
    )

    cmd_gui_dir = Path(cmd_gui_dir)
    command_files = cmd_gui_dir / "CommandFiles"
    parameter_files = cmd_gui_dir / "ParameterFiles"
    command_files.mkdir(exist_ok=True)
    parameter_files.mkdir(exist_ok=True)

    descriptions = []
    codes = []
    param_file_names = []
    skipped = []
    for command in dictionary.get_commands():
        try:
            parameters = _command_parameters(
                dictionary, command, descriptor, opcode_type
            )
        except DictionaryError as error:
            skipped.append(f"{command.name}: {error}")
            continue
        param_file_name = f"fprime_{_sanitize(command.name)}"
        with (parameter_files / param_file_name).open("wb") as file_handle:
            pickle.dump(
                (
                    [parameter.data_type for parameter in parameters],
                    [parameter.name for parameter in parameters],
                    ["" for _ in parameters],
                    [parameter.description for parameter in parameters],
                    [parameter.flag for parameter in parameters],
                    ["" for _ in parameters],
                ),
                file_handle,
                protocol=2,
            )
        descriptions.append(command.name)
        codes.append(str(function_code))
        param_file_names.append(param_file_name)

    with (command_files / COMMAND_DEFINITION_FILE).open("wb") as file_handle:
        pickle.dump((descriptions, codes, param_file_names), file_handle, protocol=2)

    with (cmd_gui_dir / COMMAND_PAGES_FILE).open("w") as file_handle:
        file_handle.write(
            "# F Prime command pages generated by fprime-cfs-gds-config\n"
            "# Description, command definition file, message id (hex), endian, Python class, address, port\n"
            f"F Prime Commands, {COMMAND_DEFINITION_FILE}, {message_id:#06x}, BE, "
            f"UdpCommands.py, {command_host}, {command_port}\n"
        )
    with (cmd_gui_dir / QUICK_BUTTONS_FILE).open("w") as file_handle:
        file_handle.write("# No quick buttons generated by fprime-cfs-gds-config\n")
    return message_id, skipped
