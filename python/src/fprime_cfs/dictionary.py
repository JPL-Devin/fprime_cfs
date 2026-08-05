"""fprime_cfs.dictionary: F Prime JSON dictionary parsing helpers

Reads the F Prime JSON dictionary and resolves the information needed to build cFS
GroundSystem configuration: primitive type layouts, dictionary constants, the ComCfg.Apid
enumeration, the single telemetry packet definition, and the command list.
"""

import json
from dataclasses import dataclass, field
from pathlib import Path
from typing import Dict, List, Optional


class DictionaryError(Exception):
    """Raised when the F Prime dictionary cannot be converted to cFS GDS configuration"""


@dataclass
class ResolvedType:
    """A dictionary type resolved down to a fixed-size primitive"""

    name: str
    kind: str  # integer | float | bool | enum
    size: int  # size in bytes
    signed: bool = False
    enum_constants: Optional[Dict[str, int]] = None


@dataclass
class Command:
    """A command entry from the dictionary"""

    name: str
    opcode: int
    params: List[dict]
    annotation: str = ""


@dataclass
class Packet:
    """The single telemetry packet definition"""

    name: str
    id: int
    members: List[str] = field(default_factory=list)


class Dictionary:
    """Parsed view of an F Prime JSON dictionary"""

    def __init__(self, path):
        with open(str(path)) as file_handle:
            self.data = json.load(file_handle)
        self.type_definitions = {
            entry["qualifiedName"]: entry
            for entry in self.data.get("typeDefinitions", [])
        }
        self.constants = {
            entry["qualifiedName"]: entry["value"]
            for entry in self.data.get("constants", [])
        }
        self.channels = {
            entry["name"]: entry for entry in self.data.get("telemetryChannels", [])
        }
        self.path = Path(path)

    def get_constant(self, qualified_name: str):
        """Get a dictionary constant by qualified name"""
        try:
            return self.constants[qualified_name]
        except KeyError:
            raise DictionaryError(
                f"Dictionary constant '{qualified_name}' not found in {self.path}. "
                "Ensure the deployment was built against a version of fprime_cfs that defines it."
            )

    def get_type_definition(self, qualified_name: str) -> dict:
        """Get a typeDefinitions entry by qualified name"""
        try:
            return self.type_definitions[qualified_name]
        except KeyError:
            raise DictionaryError(
                f"Type definition '{qualified_name}' not found in {self.path}"
            )

    def get_apid(self, name: str) -> int:
        """Get an APID value from the ComCfg.Apid enumeration"""
        apid_enum = self.type_definitions.get("ComCfg.Apid")
        if apid_enum is None:
            raise DictionaryError("ComCfg.Apid enumeration not found in dictionary")
        for constant in apid_enum.get("enumeratedConstants", []):
            if constant["name"] == name:
                return constant["value"]
        raise DictionaryError(f"APID '{name}' not found in ComCfg.Apid enumeration")

    def resolve_type(self, type_object: dict) -> ResolvedType:
        """Resolve a dictionary type object down to a fixed-size primitive

        Raises DictionaryError for types without a fixed primitive representation
        (strings, arrays, and structures).
        """
        kind = type_object.get("kind")
        name = type_object.get("name", "")
        if kind == "qualifiedIdentifier":
            definition = self.type_definitions.get(name)
            if definition is None:
                raise DictionaryError(
                    f"Type definition '{name}' not found in dictionary"
                )
            return self.resolve_definition(definition)
        if kind == "integer":
            return ResolvedType(
                name=name,
                kind="integer",
                size=type_object["size"] // 8,
                signed=type_object.get("signed", False),
            )
        if kind == "float":
            return ResolvedType(name=name, kind="float", size=type_object["size"] // 8)
        if kind == "bool":
            return ResolvedType(
                name=name, kind="bool", size=type_object.get("size", 8) // 8
            )
        raise DictionaryError(
            f"Type '{name}' of kind '{kind}' has no fixed primitive representation"
        )

    def resolve_definition(self, definition: dict) -> ResolvedType:
        """Resolve a typeDefinitions entry down to a fixed-size primitive"""
        kind = definition.get("kind")
        qualified_name = definition.get("qualifiedName", "")
        if kind == "alias":
            return self.resolve_type(definition["underlyingType"])
        if kind == "enum":
            representation = self.resolve_type(definition["representationType"])
            enum_constants = {
                constant["name"]: constant["value"]
                for constant in definition.get("enumeratedConstants", [])
            }
            return ResolvedType(
                name=qualified_name,
                kind="enum",
                size=representation.size,
                signed=representation.signed,
                enum_constants=enum_constants,
            )
        raise DictionaryError(
            f"Type '{qualified_name}' of kind '{kind}' has no fixed primitive representation"
        )

    def get_packet(self) -> Packet:
        """Get the single telemetry packet defined by the dictionary

        Raises DictionaryError unless exactly one packet is defined across all packet sets.
        """
        packets = []
        for packet_set in self.data.get("telemetryPacketSets", []):
            for packet in packet_set.get("members", []):
                packets.append(packet)
        if len(packets) != 1:
            names = ", ".join(packet.get("name", "?") for packet in packets) or "none"
            raise DictionaryError(
                f"cFS GDS configuration requires exactly one telemetry packet; "
                f"found {len(packets)} ({names})"
            )
        packet = packets[0]
        return Packet(
            name=packet["name"], id=packet["id"], members=list(packet["members"])
        )

    def get_channel_type(self, channel_name: str) -> ResolvedType:
        """Resolve the type of a telemetry channel by qualified name"""
        channel = self.channels.get(channel_name)
        if channel is None:
            raise DictionaryError(
                f"Telemetry channel '{channel_name}' not found in dictionary"
            )
        return self.resolve_type(channel["type"])

    def get_commands(self) -> List[Command]:
        """Get the command list from the dictionary"""
        return [
            Command(
                name=entry["name"],
                opcode=entry["opcode"],
                params=entry.get("formalParams", []),
                annotation=entry.get("annotation", ""),
            )
            for entry in self.data.get("commands", [])
        ]
