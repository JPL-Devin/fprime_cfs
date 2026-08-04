// ======================================================================
// \title  CfsTlmDeframerTester.cpp
// \brief  cpp file for CfsTlmDeframer component test harness implementation class
// ======================================================================

#include "CfsTlmDeframerTester.hpp"
#include <cstring>

namespace FPrimeCfs {

// ----------------------------------------------------------------------
// Construction and destruction
// ----------------------------------------------------------------------

CfsTlmDeframerTester ::CfsTlmDeframerTester()
    : CfsTlmDeframerGTestBase("CfsTlmDeframerTester", CfsTlmDeframerTester::MAX_HISTORY_SIZE),
      component("CfsTlmDeframer") {
    this->initComponents();
    this->connectPorts();
}

CfsTlmDeframerTester ::~CfsTlmDeframerTester() {}

// ----------------------------------------------------------------------
// Helpers
// ----------------------------------------------------------------------

FwSizeType CfsTlmDeframerTester ::makePacket(U8* dest, bool command, bool secHdr, FwSizeType payloadSize, U8 fill) {
    dest[0] = static_cast<U8>((command ? (CFS_TLM_DEFRAMER_SPACE_PACKET_TYPE_MASK >> 8) : 0) |
                              (secHdr ? (CFS_TLM_DEFRAMER_SPACE_PACKET_SEC_HDR_MASK >> 8) : 0));
    dest[1] = 0x01;  // APID low byte
    dest[2] = 0xC0;  // Sequence flags: unsegmented
    dest[3] = 0x00;  // Sequence count
    const FwSizeType lengthToken = payloadSize - 1;
    dest[CFS_TLM_DEFRAMER_SPACE_PACKET_LENGTH_OFFSET] = static_cast<U8>((lengthToken >> 8) & 0xFF);
    dest[CFS_TLM_DEFRAMER_SPACE_PACKET_LENGTH_OFFSET + 1] = static_cast<U8>(lengthToken & 0xFF);
    for (FwSizeType i = 0; i < payloadSize; i++) {
        dest[CFS_TLM_DEFRAMER_SPACE_PACKET_HEADER_SIZE + i] = static_cast<U8>(fill + i);
    }
    return CFS_TLM_DEFRAMER_SPACE_PACKET_HEADER_SIZE + payloadSize;
}

void CfsTlmDeframerTester ::sendData(U8* bytes, FwSizeType size) {
    Fw::Buffer buffer(bytes, size);
    ComCfg::FrameContext context;
    this->invoke_to_dataIn(0, buffer, context);
}

// ----------------------------------------------------------------------
// Tests
// ----------------------------------------------------------------------

void CfsTlmDeframerTester ::testStripTelemetrySecHdr() {
    U8 packet[64] = {};
    U8 original[64] = {};
    // Payload: 6-byte secondary header (time) followed by 8 user data bytes
    const FwSizeType size = this->makePacket(packet, false, true, CFS_TLM_DEFRAMER_TLM_SEC_HDR_SIZE + 8, 0x40);
    (void)std::memcpy(original, packet, size);
    this->sendData(packet, size);

    ASSERT_from_dataOut_SIZE(1);
    const Fw::Buffer& out = this->fromPortHistory_dataOut->at(0).data;
    ASSERT_EQ(out.getData(), packet);  // stripped in place
    ASSERT_EQ(out.getSize(), size - CFS_TLM_DEFRAMER_TLM_SEC_HDR_SIZE);
    // Secondary header flag cleared
    ASSERT_EQ(packet[0] & static_cast<U8>(CFS_TLM_DEFRAMER_SPACE_PACKET_SEC_HDR_MASK >> 8), 0);
    // Length token shortened by the secondary header size
    const FwSizeType lengthToken =
        (static_cast<FwSizeType>(packet[CFS_TLM_DEFRAMER_SPACE_PACKET_LENGTH_OFFSET]) << 8) |
        static_cast<FwSizeType>(packet[CFS_TLM_DEFRAMER_SPACE_PACKET_LENGTH_OFFSET + 1]);
    ASSERT_EQ(lengthToken, 8 - 1);
    // Payload preserved: user data follows the primary header directly
    ASSERT_EQ(std::memcmp(&packet[CFS_TLM_DEFRAMER_SPACE_PACKET_HEADER_SIZE],
                          &original[CFS_TLM_DEFRAMER_SPACE_PACKET_HEADER_SIZE + CFS_TLM_DEFRAMER_TLM_SEC_HDR_SIZE], 8),
              0);
}

void CfsTlmDeframerTester ::testPassthroughBareTelemetry() {
    U8 packet[64] = {};
    U8 original[64] = {};
    const FwSizeType size = this->makePacket(packet, false, false, 10, 0x20);
    (void)std::memcpy(original, packet, size);
    this->sendData(packet, size);

    ASSERT_from_dataOut_SIZE(1);
    const Fw::Buffer& out = this->fromPortHistory_dataOut->at(0).data;
    ASSERT_EQ(out.getSize(), size);
    ASSERT_EQ(std::memcmp(out.getData(), original, size), 0);
}

void CfsTlmDeframerTester ::testPassthroughCommandPacket() {
    U8 packet[64] = {};
    U8 original[64] = {};
    const FwSizeType size = this->makePacket(packet, true, true, 10, 0x30);
    (void)std::memcpy(original, packet, size);
    this->sendData(packet, size);

    ASSERT_from_dataOut_SIZE(1);
    const Fw::Buffer& out = this->fromPortHistory_dataOut->at(0).data;
    ASSERT_EQ(out.getSize(), size);
    ASSERT_EQ(std::memcmp(out.getData(), original, size), 0);
}

void CfsTlmDeframerTester ::testPassthroughShortPacket() {
    // Telemetry packet with secondary header flag set but a data field no larger than
    // the secondary header itself: forwarded unmodified
    U8 packet[64] = {};
    U8 original[64] = {};
    const FwSizeType size = this->makePacket(packet, false, true, CFS_TLM_DEFRAMER_TLM_SEC_HDR_SIZE, 0x50);
    (void)std::memcpy(original, packet, size);
    this->sendData(packet, size);

    ASSERT_from_dataOut_SIZE(1);
    const Fw::Buffer& out = this->fromPortHistory_dataOut->at(0).data;
    ASSERT_EQ(out.getSize(), size);
    ASSERT_EQ(std::memcmp(out.getData(), original, size), 0);
}

void CfsTlmDeframerTester ::testDataReturnPassthrough() {
    U8 storage[8] = {};
    Fw::Buffer buffer(storage, sizeof(storage));
    ComCfg::FrameContext context;
    this->invoke_to_dataReturnIn(0, buffer, context);
    ASSERT_from_dataReturnOut_SIZE(1);
    ASSERT_EQ(this->fromPortHistory_dataReturnOut->at(0).data.getData(), storage);
}

}  // namespace FPrimeCfs
