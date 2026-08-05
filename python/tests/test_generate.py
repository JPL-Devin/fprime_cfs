"""Tests for the fprime_cfs_gds configuration generators"""

import copy
import json
import pickle

import pytest

from fprime_cfs_gds.comm import split_space_packets
from fprime_cfs_gds.commands import generate_commands
from fprime_cfs_gds.dictionary import Dictionary, DictionaryError
from fprime_cfs_gds.telemetry import generate_telemetry

BASE_DICTIONARY = {
    "metadata": {"deploymentName": "Test"},
    "typeDefinitions": [
        {
            "kind": "alias",
            "qualifiedName": "FwPacketDescriptorType",
            "type": {"name": "U16", "kind": "integer", "size": 16, "signed": False},
            "underlyingType": {
                "name": "U16",
                "kind": "integer",
                "size": 16,
                "signed": False,
            },
        },
        {
            "kind": "alias",
            "qualifiedName": "FwTlmPacketizeIdType",
            "type": {"name": "U16", "kind": "integer", "size": 16, "signed": False},
            "underlyingType": {
                "name": "U16",
                "kind": "integer",
                "size": 16,
                "signed": False,
            },
        },
        {
            "kind": "alias",
            "qualifiedName": "FwOpcodeType",
            "type": {"name": "FwIdType", "kind": "qualifiedIdentifier"},
            "underlyingType": {
                "name": "U32",
                "kind": "integer",
                "size": 32,
                "signed": False,
            },
        },
        {
            "kind": "enum",
            "qualifiedName": "ComCfg.Apid",
            "representationType": {
                "name": "U16",
                "kind": "integer",
                "size": 16,
                "signed": False,
            },
            "enumeratedConstants": [
                {"name": "FW_PACKET_COMMAND", "value": 0},
                {"name": "FW_PACKET_PACKETIZED_TLM", "value": 4},
            ],
        },
        {
            "kind": "enum",
            "qualifiedName": "Test.Mode",
            "representationType": {
                "name": "U8",
                "kind": "integer",
                "size": 8,
                "signed": False,
            },
            "enumeratedConstants": [
                {"name": "OFF", "value": 0},
                {"name": "ON", "value": 1},
            ],
        },
    ],
    "constants": [
        {"kind": "constant", "qualifiedName": "ComCfg.SpacecraftId", "value": 68},
        {"kind": "constant", "qualifiedName": "ComCfg.TmFrameFixedSize", "value": 1024},
        {
            "kind": "constant",
            "qualifiedName": "ComCfg.FprimeCommandFunctionCode",
            "value": 0,
        },
    ],
    "telemetryChannels": [
        {
            "name": "comp.counter",
            "type": {"name": "U32", "kind": "integer", "size": 32, "signed": False},
            "id": 1,
        },
        {
            "name": "comp.flag",
            "type": {"name": "U8", "kind": "integer", "size": 8, "signed": False},
            "id": 2,
        },
        {
            "name": "comp.mode",
            "type": {"name": "Test.Mode", "kind": "qualifiedIdentifier"},
            "id": 3,
        },
    ],
    "commands": [
        {
            "name": "comp.NO_OP",
            "commandKind": "async",
            "opcode": 256,
            "formalParams": [],
        },
        {
            "name": "comp.SET_MODE",
            "commandKind": "async",
            "opcode": 257,
            "formalParams": [
                {
                    "name": "mode",
                    "type": {"name": "Test.Mode", "kind": "qualifiedIdentifier"},
                },
            ],
        },
        {
            "name": "comp.NO_OP_STRING",
            "commandKind": "async",
            "opcode": 258,
            "formalParams": [
                {
                    "name": "text",
                    "type": {"name": "string", "kind": "string", "size": 40},
                },
            ],
        },
    ],
    "telemetryPacketSets": [
        {
            "name": "TestPackets",
            "members": [
                {
                    "name": "Health",
                    "id": 1,
                    "group": 1,
                    "members": ["comp.counter", "comp.flag", "comp.mode"],
                },
            ],
        },
    ],
}


def make_dictionary(tmp_path, data):
    path = tmp_path / "dictionary.json"
    path.write_text(json.dumps(data))
    return Dictionary(path)


def test_telemetry_generation(tmp_path):
    dictionary = make_dictionary(tmp_path, BASE_DICTIONARY)
    stream_id = generate_telemetry(dictionary, tmp_path)
    assert stream_id == 4

    pages = (tmp_path / "telemetry-pages.txt").read_text().splitlines()
    data_lines = [line for line in pages if not line.startswith("#")]
    assert data_lines == ["Health, GenericTelemetry.py, 0x0004, fprime-tlm.txt"]

    rows = [
        [column.strip() for column in line.split(",")]
        for line in (tmp_path / "fprime-tlm.txt").read_text().splitlines()
        if not line.startswith("#")
    ]
    # Datagram layout: 6 header + 2 descriptor + 2 id + 11 time + values, minus the
    # 4-byte offset applied by the GUI
    assert rows[0][:4] == ["Packet Id", "4", "2", ">H"]
    assert rows[1][:4] == ["Time Seconds", "9", "4", ">I"]
    assert rows[2][:4] == ["Time Microseconds", "13", "4", ">I"]
    assert rows[3][:5] == ["comp.counter", "17", "4", ">I", "Dec"]
    assert rows[4][:5] == ["comp.flag", "21", "1", "B", "Dec"]
    assert rows[5][:9] == [
        "comp.mode",
        "22",
        "1",
        "B",
        "Enm",
        "OFF",
        "ON",
        "NULL",
        "NULL",
    ]


def test_multiple_packets_error(tmp_path):
    data = copy.deepcopy(BASE_DICTIONARY)
    data["telemetryPacketSets"][0]["members"].append(
        {"name": "Extra", "id": 2, "group": 1, "members": []}
    )
    dictionary = make_dictionary(tmp_path, data)
    with pytest.raises(DictionaryError, match="exactly one telemetry packet"):
        generate_telemetry(dictionary, tmp_path)


def test_no_packets_error(tmp_path):
    data = copy.deepcopy(BASE_DICTIONARY)
    data["telemetryPacketSets"] = []
    dictionary = make_dictionary(tmp_path, data)
    with pytest.raises(DictionaryError, match="exactly one telemetry packet"):
        generate_telemetry(dictionary, tmp_path)


def test_command_generation(tmp_path):
    dictionary = make_dictionary(tmp_path, BASE_DICTIONARY)
    message_id, skipped = generate_commands(dictionary, tmp_path)
    assert message_id == 0x1800
    assert len(skipped) == 1 and "comp.NO_OP_STRING" in skipped[0]

    pages = [
        line
        for line in (tmp_path / "command-pages.txt").read_text().splitlines()
        if not line.startswith("#")
    ]
    assert pages == [
        "F Prime Commands, fprime_cmds, 0x1800, BE, UdpCommands.py, 127.0.0.1, 1234"
    ]

    with (tmp_path / "CommandFiles" / "fprime_cmds").open("rb") as file_handle:
        descriptions, codes, param_files = pickle.load(file_handle)
    assert descriptions == ["comp.NO_OP", "comp.SET_MODE"]
    # Every command carries the single dictionary function code
    assert codes == ["0", "0"]

    with (tmp_path / "ParameterFiles" / param_files[1]).open("rb") as file_handle:
        data_types, names, _, descriptions, flags, string_lengths = pickle.load(
            file_handle
        )
    # The framing descriptor and opcode are prepended ahead of the command arguments
    assert names == ["FramingDescriptor", "Opcode", "mode"]
    assert flags == ["--uint16", "--uint32", "--uint8"]
    assert "Enter 0 (FW_PACKET_COMMAND)" in descriptions[0]
    assert "Enter 257 (0x101)" in descriptions[1]
    assert "OFF=0; ON=1" in descriptions[2].replace(",", ";")


def test_missing_function_code_error(tmp_path):
    data = copy.deepcopy(BASE_DICTIONARY)
    data["constants"] = [
        c
        for c in data["constants"]
        if c["qualifiedName"] != "ComCfg.FprimeCommandFunctionCode"
    ]
    dictionary = make_dictionary(tmp_path, data)
    with pytest.raises(DictionaryError, match="FprimeCommandFunctionCode"):
        generate_commands(dictionary, tmp_path)


def test_split_space_packets():
    def packet(apid, payload):
        return (
            bytes([apid >> 8, apid & 0xFF, 0xC0, 0x00, 0x00, len(payload) - 1])
            + payload
        )

    telemetry = packet(0x0004, b"\x01\x02\x03")
    idle = packet(0x07FF, b"\x00\x00")
    assert split_space_packets(telemetry + idle + telemetry) == [telemetry, telemetry]
    # Truncated trailing packets are dropped
    assert split_space_packets(telemetry + telemetry[:-1]) == [telemetry]
