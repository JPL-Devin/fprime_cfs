"""fprime_cfs.comm: communication bridge between the GdsBridge app and the cFS GroundSystem

The fprime_gds GdsBridge cFS application exposes a TCP server carrying CCSDS TM frames on
downlink and expecting CCSDS TC frames on uplink. The cFS GroundSystem exchanges bare
space packets over UDP: telemetry datagrams in, command datagrams out.

This bridge:
- connects to the GdsBridge TCP server, deframes fixed-size TM frames, splits the frame
  contents into space packets (dropping idle packets), and forwards each complete packet
  as a UDP datagram to the GroundSystem telemetry port;
- listens on the GroundSystem command UDP port, wraps each received cFS command packet in
  a CCSDS TC frame, and sends it over the TCP connection.
"""

import argparse
import socket
import struct
import sys
import threading
import time

from fprime_gds.common.communication.ccsds.space_data_link import (
    SpaceDataLinkFramerDeframer,
)

from .dictionary import Dictionary

SPACE_PACKET_HEADER_SIZE = 6
APID_MASK = 0x07FF
IDLE_APID = 0x7FF
RECONNECT_PERIOD_SECONDS = 1.0
# Maximum space packet size wrappable in a TC frame: 1024-byte max frame length minus
# the 5-byte TC primary header and 2-byte FECF trailer, with margin for the length field
TC_MAX_PACKET_SIZE = 1016


def split_space_packets(data: bytes):
    """Split TM frame contents into complete space packets, dropping idle data

    Returns a list of complete space packets, headers included.
    """
    packets = []
    offset = 0
    while offset + SPACE_PACKET_HEADER_SIZE <= len(data):
        stream_id, _, length_token = struct.unpack_from(">HHH", data, offset)
        packet_length = SPACE_PACKET_HEADER_SIZE + length_token + 1
        if offset + packet_length > len(data):
            break
        if (stream_id & APID_MASK) != IDLE_APID:
            packets.append(data[offset : offset + packet_length])
        offset += packet_length
    return packets


class GroundSystemBridge:
    """Bidirectional bridge between the GdsBridge TCP server and the cFS GroundSystem"""

    def __init__(
        self,
        gds_address,
        gds_port,
        telemetry_address="127.0.0.1",
        telemetry_port=2234,
        command_address="127.0.0.1",
        command_port=1234,
        scid=None,
        vcid=1,
        frame_size=None,
    ):
        self.gds_address = gds_address
        self.gds_port = gds_port
        self.telemetry_destination = (telemetry_address, telemetry_port)
        self.command_port = command_port
        self.framer = SpaceDataLinkFramerDeframer(scid, vcid, frame_size)
        self.telemetry_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.command_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.command_socket.bind((command_address, command_port))
        self.tcp_socket = None
        self.tcp_lock = threading.Lock()
        self.running = True

    def connect(self):
        """Connect to the GdsBridge TCP server, retrying until successful"""
        while self.running:
            try:
                connection = socket.create_connection((self.gds_address, self.gds_port))
                with self.tcp_lock:
                    if self.tcp_socket is not None:
                        self.tcp_socket.close()
                    self.tcp_socket = connection
                print(
                    f"[INFO] Connected to GdsBridge at {self.gds_address}:{self.gds_port}"
                )
                return connection
            except OSError:
                time.sleep(RECONNECT_PERIOD_SECONDS)
        return None

    def downlink(self):
        """Receive TM frames over TCP and forward space packets to the GroundSystem"""
        connection = self.connect()
        pool = b""
        while self.running and connection is not None:
            try:
                data = connection.recv(4096)
            except OSError:
                data = b""
            if not data:
                if not self.running:
                    break
                print(
                    "[WARNING] GdsBridge connection lost; reconnecting", file=sys.stderr
                )
                connection = self.connect()
                pool = b""
                continue
            pool += data
            while True:
                frame, pool, _ = self.framer.deframe(pool, no_copy=True)
                if frame is None:
                    break
                for packet in split_space_packets(frame):
                    self.telemetry_socket.sendto(packet, self.telemetry_destination)

    def uplink(self):
        """Receive command packets over UDP and send TC frames to the GdsBridge"""
        while self.running:
            try:
                packet, _ = self.command_socket.recvfrom(4096)
            except OSError:
                continue
            if not packet:
                continue
            if len(packet) > TC_MAX_PACKET_SIZE:
                print(
                    f"[WARNING] Dropping command: {len(packet)} byte packet exceeds "
                    f"the {TC_MAX_PACKET_SIZE} byte TC frame limit",
                    file=sys.stderr,
                )
                continue
            try:
                framed = self.framer.frame(packet)
            except (AssertionError, ValueError) as error:
                print(f"[WARNING] Dropping command: {error}", file=sys.stderr)
                continue
            with self.tcp_lock:
                connection = self.tcp_socket
            if connection is None:
                print(
                    "[WARNING] Dropping command: GdsBridge not connected",
                    file=sys.stderr,
                )
                continue
            try:
                connection.sendall(framed)
            except OSError as error:
                print(f"[WARNING] Dropping command: {error}", file=sys.stderr)

    def start(self):
        """Start the downlink and uplink threads"""
        threads = [
            threading.Thread(target=self.downlink, daemon=True),
            threading.Thread(target=self.uplink, daemon=True),
        ]
        for thread in threads:
            thread.start()
        return threads

    def stop(self):
        """Stop the bridge"""
        self.running = False
        with self.tcp_lock:
            if self.tcp_socket is not None:
                try:
                    self.tcp_socket.shutdown(socket.SHUT_RDWR)
                except OSError:
                    pass
                self.tcp_socket.close()
        self.telemetry_socket.close()
        self.command_socket.close()


# Bridge arguments in the fprime_gds ParserBase specification format
BRIDGE_ARGUMENTS = {
    ("--gds-address",): {
        "action": "store",
        "default": "127.0.0.1",
        "type": str,
        "help": "GdsBridge TCP server address. Default: %(default)s",
    },
    ("--gds-port",): {
        "action": "store",
        "default": 15010,
        "type": int,
        "help": "GdsBridge TCP server port. Default: %(default)s",
    },
    ("--telemetry-address",): {
        "action": "store",
        "default": "127.0.0.1",
        "type": str,
        "help": "GroundSystem telemetry UDP address to send to. Default: %(default)s",
    },
    ("--telemetry-port",): {
        "action": "store",
        "default": 2234,
        "type": int,
        "help": "GroundSystem telemetry UDP port. Default: %(default)s",
    },
    ("--command-address",): {
        "action": "store",
        "default": "127.0.0.1",
        "type": str,
        "help": "GroundSystem command UDP address to listen on. Default: %(default)s",
    },
    ("--command-port",): {
        "action": "store",
        "default": 1234,
        "type": int,
        "help": "GroundSystem command UDP port to listen on. Default: %(default)s",
    },
    ("--vcid",): {
        "action": "store",
        "default": 1,
        "type": lambda value: int(value, 0),
        "help": "CCSDS virtual channel id. Default: %(default)s",
    },
}


def add_arguments(parser: argparse.ArgumentParser):
    """Add bridge arguments to an argument parser"""
    for flags, specification in BRIDGE_ARGUMENTS.items():
        parser.add_argument(*flags, **specification)


def bridge_from_arguments(args, dictionary: Dictionary) -> GroundSystemBridge:
    """Construct a bridge from parsed arguments and dictionary constants"""
    return GroundSystemBridge(
        gds_address=args.gds_address,
        gds_port=args.gds_port,
        telemetry_address=args.telemetry_address,
        telemetry_port=args.telemetry_port,
        command_address=args.command_address,
        command_port=args.command_port,
        scid=dictionary.get_constant("ComCfg.SpacecraftId"),
        vcid=args.vcid,
        frame_size=dictionary.get_constant("ComCfg.TmFrameFixedSize"),
    )


def main():
    """Entry point running the bridge standalone"""
    parser = argparse.ArgumentParser(
        description="Bridge between the fprime_gds GdsBridge TCP server and the cFS GroundSystem"
    )
    parser.add_argument(
        "--dictionary", required=True, help="Path to the F Prime JSON dictionary"
    )
    add_arguments(parser)
    args = parser.parse_args()
    bridge = bridge_from_arguments(args, Dictionary(args.dictionary))
    threads = bridge.start()
    print("[INFO] fprime-cfs-comm bridge running. CTRL-C to exit.")
    try:
        while all(thread.is_alive() for thread in threads):
            time.sleep(1)
    except KeyboardInterrupt:
        pass
    bridge.stop()
    return 0


if __name__ == "__main__":
    sys.exit(main())
