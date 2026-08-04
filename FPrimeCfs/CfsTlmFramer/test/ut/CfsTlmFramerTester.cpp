// ======================================================================
// \title  CfsTlmFramerTester.cpp
// \brief  cpp file for CfsTlmFramer component test harness implementation class
// ======================================================================

#include "CfsTlmFramerTester.hpp"
#include <cstring>

namespace FPrimeCfs {

// Test time: 0x01020304 seconds, 500000 microseconds = 0x8000 subseconds (1/65536 s)
static const U32 TEST_SECONDS = 0x01020304;
static const U32 TEST_USECONDS = 500000;
static const U8 EXPECTED_TIME_BYTES[CFS_TLM_FRAMER_TLM_SEC_HDR_SIZE] = {0x01, 0x02, 0x03, 0x04, 0x80, 0x00};

// ----------------------------------------------------------------------
// Construction and destruction
// ----------------------------------------------------------------------

CfsTlmFramerTester ::CfsTlmFramerTester()
    : CfsTlmFramerGTestBase("CfsTlmFramerTester", CfsTlmFramerTester::MAX_HISTORY_SIZE),
      component("CfsTlmFramer") {
    this->initComponents();
    this->connectPorts();
    Fw::Time testTime(TEST_SECONDS, TEST_USECONDS);
    this->setTestTime(testTime);
}

CfsTlmFramerTester ::~CfsTlmFramerTester() {}

// ----------------------------------------------------------------------
// Helpers
// ----------------------------------------------------------------------

FwSizeType CfsTlmFramerTester ::makePacket(U8* dest, bool command, bool secHdr, FwSizeType payloadSize, U8 fill) {
    dest[0] = static_cast<U8>((command ? (CFS_TLM_FRAMER_SPACE_PACKET_TYPE_MASK >> 8) : 0) |
                              (secHdr ? (CFS_TLM_FRAMER_SPACE_PACKET_SEC_HDR_MASK >> 8) : 0));
    dest[1] = 0x01;  // APID low byte
    dest[2] = 0xC0;  // Sequence flags: unsegmented
    dest[3] = 0x00;  // Sequence count
    const FwSizeType lengthToken = payloadSize - 1;
    dest[CFS_TLM_FRAMER_SPACE_PACKET_LENGTH_OFFSET] = static_cast<U8>((lengthToken >> 8) & 0xFF);
    dest[CFS_TLM_FRAMER_SPACE_PACKET_LENGTH_OFFSET + 1] = static_cast<U8>(lengthToken & 0xFF);
    for (FwSizeType i = 0; i < payloadSize; i++) {
        dest[CFS_TLM_FRAMER_SPACE_PACKET_HEADER_SIZE + i] = static_cast<U8>(fill + i);
    }
    return CFS_TLM_FRAMER_SPACE_PACKET_HEADER_SIZE + payloadSize;
}

void CfsTlmFramerTester ::sendData(U8* bytes, FwSizeType size) {
    Fw::Buffer buffer(bytes, size);
    ComCfg::FrameContext context;
    this->invoke_to_dataIn(0, buffer, context);
}

Fw::Buffer CfsTlmFramerTester ::from_bufferAllocate_handler(FwIndexType portNum, FwSizeType size) {
    return Fw::Buffer(this->m_allocStorage, FW_MIN(size, this->m_allocLimit));
}

// ----------------------------------------------------------------------
// Tests
// ----------------------------------------------------------------------

void CfsTlmFramerTester ::testFrameTelemetryPacket() {
    U8 packet[64] = {};
    const FwSizeType size = this->makePacket(packet, false, false, 10, 0x40);
    this->sendData(packet, size);

    // Framed output carries the secondary header flag, extended length, time, and payload
    ASSERT_from_dataOut_SIZE(1);
    const Fw::Buffer& framed = this->fromPortHistory_dataOut->at(0).data;
    ASSERT_EQ(framed.getSize(), size + CFS_TLM_FRAMER_TLM_SEC_HDR_SIZE);
    const U8* const out = framed.getData();
    ASSERT_EQ(out[0] & static_cast<U8>(CFS_TLM_FRAMER_SPACE_PACKET_SEC_HDR_MASK >> 8),
              static_cast<U8>(CFS_TLM_FRAMER_SPACE_PACKET_SEC_HDR_MASK >> 8));
    const FwSizeType lengthToken = (static_cast<FwSizeType>(out[CFS_TLM_FRAMER_SPACE_PACKET_LENGTH_OFFSET]) << 8) |
                                   static_cast<FwSizeType>(out[CFS_TLM_FRAMER_SPACE_PACKET_LENGTH_OFFSET + 1]);
    ASSERT_EQ(lengthToken, (10 - 1) + CFS_TLM_FRAMER_TLM_SEC_HDR_SIZE);
    ASSERT_EQ(std::memcmp(&out[CFS_TLM_FRAMER_SPACE_PACKET_HEADER_SIZE], EXPECTED_TIME_BYTES,
                          CFS_TLM_FRAMER_TLM_SEC_HDR_SIZE),
              0);
    ASSERT_EQ(std::memcmp(&out[CFS_TLM_FRAMER_SPACE_PACKET_HEADER_SIZE + CFS_TLM_FRAMER_TLM_SEC_HDR_SIZE],
                          &packet[CFS_TLM_FRAMER_SPACE_PACKET_HEADER_SIZE], 10),
              0);
    // Original buffer returned upstream
    ASSERT_from_dataReturnOut_SIZE(1);
    ASSERT_EQ(this->fromPortHistory_dataReturnOut->at(0).data.getData(), packet);
}

void CfsTlmFramerTester ::testPassthroughCommandPacket() {
    U8 packet[64] = {};
    const FwSizeType size = this->makePacket(packet, true, false, 8, 0x10);
    this->sendData(packet, size);

    ASSERT_from_dataOut_SIZE(1);
    const Fw::Buffer& framed = this->fromPortHistory_dataOut->at(0).data;
    ASSERT_EQ(framed.getSize(), size);
    ASSERT_EQ(std::memcmp(framed.getData(), packet, size), 0);
    ASSERT_from_dataReturnOut_SIZE(1);
}

void CfsTlmFramerTester ::testPassthroughSecHdrTelemetry() {
    U8 packet[64] = {};
    const FwSizeType size = this->makePacket(packet, false, true, 12, 0x20);
    this->sendData(packet, size);

    ASSERT_from_dataOut_SIZE(1);
    const Fw::Buffer& framed = this->fromPortHistory_dataOut->at(0).data;
    ASSERT_EQ(framed.getSize(), size);
    ASSERT_EQ(std::memcmp(framed.getData(), packet, size), 0);
    ASSERT_from_dataReturnOut_SIZE(1);
}

void CfsTlmFramerTester ::testMultiplePackets() {
    U8 buffer[128] = {};
    const FwSizeType first = this->makePacket(&buffer[0], false, false, 5, 0x50);
    const FwSizeType second = this->makePacket(&buffer[first], true, false, 7, 0x60);
    const FwSizeType third = this->makePacket(&buffer[first + second], false, true, 9, 0x70);
    const FwSizeType total = first + second + third;
    this->sendData(buffer, total);

    ASSERT_from_dataOut_SIZE(1);
    const Fw::Buffer& framed = this->fromPortHistory_dataOut->at(0).data;
    ASSERT_EQ(framed.getSize(), total + CFS_TLM_FRAMER_TLM_SEC_HDR_SIZE);
    const U8* const out = framed.getData();
    // First packet framed: flag set and payload after the inserted secondary header
    ASSERT_NE(out[0] & static_cast<U8>(CFS_TLM_FRAMER_SPACE_PACKET_SEC_HDR_MASK >> 8), 0);
    ASSERT_EQ(std::memcmp(&out[CFS_TLM_FRAMER_SPACE_PACKET_HEADER_SIZE + CFS_TLM_FRAMER_TLM_SEC_HDR_SIZE],
                          &buffer[CFS_TLM_FRAMER_SPACE_PACKET_HEADER_SIZE], 5),
              0);
    // Second and third packets copied verbatim after the framed first packet
    ASSERT_EQ(std::memcmp(&out[first + CFS_TLM_FRAMER_TLM_SEC_HDR_SIZE], &buffer[first], second + third), 0);
    ASSERT_from_dataReturnOut_SIZE(1);
}

void CfsTlmFramerTester ::testMalformedBuffer() {
    // Length token indicates more data than the buffer holds
    U8 packet[64] = {};
    (void)this->makePacket(packet, false, false, 20, 0x30);
    this->sendData(packet, 12);

    // Buffer forwarded verbatim
    ASSERT_from_dataOut_SIZE(1);
    const Fw::Buffer& framed = this->fromPortHistory_dataOut->at(0).data;
    ASSERT_EQ(framed.getSize(), 12);
    ASSERT_EQ(std::memcmp(framed.getData(), packet, 12), 0);
    ASSERT_from_dataReturnOut_SIZE(1);
}

void CfsTlmFramerTester ::testAllocationFailure() {
    this->m_allocLimit = 4;  // Force an undersized allocation
    U8 packet[64] = {};
    const FwSizeType size = this->makePacket(packet, false, false, 10, 0x40);
    this->sendData(packet, size);

    // No framed output; original buffer returned; undersized allocation deallocated; status flows
    ASSERT_from_dataOut_SIZE(0);
    ASSERT_from_dataReturnOut_SIZE(1);
    ASSERT_EQ(this->fromPortHistory_dataReturnOut->at(0).data.getData(), packet);
    ASSERT_from_bufferDeallocate_SIZE(1);
    ASSERT_from_comStatusOut_SIZE(1);
    ASSERT_EQ(this->fromPortHistory_comStatusOut->at(0).condition, Fw::Success::SUCCESS);
}

void CfsTlmFramerTester ::testDataReturnDeallocate() {
    U8 storage[8] = {};
    Fw::Buffer buffer(storage, sizeof(storage));
    ComCfg::FrameContext context;
    this->invoke_to_dataReturnIn(0, buffer, context);
    ASSERT_from_bufferDeallocate_SIZE(1);
    ASSERT_EQ(this->fromPortHistory_bufferDeallocate->at(0).fwBuffer.getData(), storage);
}

void CfsTlmFramerTester ::testComStatusPassthrough() {
    Fw::Success status = Fw::Success::SUCCESS;
    this->invoke_to_comStatusIn(0, status);
    ASSERT_from_comStatusOut_SIZE(1);
    ASSERT_EQ(this->fromPortHistory_comStatusOut->at(0).condition, Fw::Success::SUCCESS);
}

}  // namespace FPrimeCfs
