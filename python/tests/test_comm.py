"""Tests for the fprime_cfs GroundSystem communication bridge"""

import socket
import threading

from fprime_cfs.comm import TC_MAX_PACKET_SIZE, GroundSystemBridge


class StubFramer:
    """Stub framer recording framed packets"""

    def frame(self, packet):
        return b"FRAME" + packet

    def deframe(self, pool, no_copy=False):
        return None, pool, 0


def make_bridge():
    bridge = GroundSystemBridge(
        "127.0.0.1", 0, command_port=0, scid=68, frame_size=1024
    )
    bridge.framer = StubFramer()
    return bridge


def test_uplink_frames_commands_and_survives_oversized_packets():
    bridge = make_bridge()
    command_port = bridge.command_socket.getsockname()[1]
    local, remote = socket.socketpair()
    try:
        bridge.tcp_socket = local
        thread = threading.Thread(target=bridge.uplink, daemon=True)
        thread.start()

        sender = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        sender.sendto(b"\x01\x02\x03", ("127.0.0.1", command_port))
        assert remote.recv(4096) == b"FRAME\x01\x02\x03"

        # Oversized packets are dropped without killing the uplink thread
        sender.sendto(b"x" * (TC_MAX_PACKET_SIZE + 1), ("127.0.0.1", command_port))
        sender.sendto(b"\x04\x05", ("127.0.0.1", command_port))
        assert remote.recv(4096) == b"FRAME\x04\x05"
        assert thread.is_alive()
        sender.close()
    finally:
        bridge.stop()
        remote.close()


def test_stop_closes_sockets():
    bridge = make_bridge()
    bridge.stop()
    assert bridge.running is False
    assert bridge.command_socket.fileno() == -1
    assert bridge.telemetry_socket.fileno() == -1
