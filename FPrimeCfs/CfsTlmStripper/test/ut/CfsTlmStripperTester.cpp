// ======================================================================
// \title  CfsTlmStripperTester.cpp
// \brief  cpp file for CfsTlmStripper component test harness implementation class
// ======================================================================

#include "CfsTlmStripperTester.hpp"
#include "STest/Random/Random.hpp"
#include <cstring>

namespace FPrimeCfs {

// ----------------------------------------------------------------------
// Construction and destruction
// ----------------------------------------------------------------------

CfsTlmStripperTester ::CfsTlmStripperTester()
    : CfsTlmStripperGTestBase("CfsTlmStripperTester", CfsTlmStripperTester::MAX_HISTORY_SIZE),
      component("CfsTlmStripper") {
    this->initComponents();
    this->connectPorts();
}

CfsTlmStripperTester ::~CfsTlmStripperTester() {}

// ----------------------------------------------------------------------
// Helpers
// ----------------------------------------------------------------------

Fw::Buffer CfsTlmStripperTester ::buildPacket(bool command, bool secHdr, FwSizeType payloadSize) {
    const FwSizeType secHdrSize = secHdr ? CFS_TLM_STRIPPER_SEC_HDR_SIZE : 0;
    const FwSizeType totalSize = CFS_TLM_STRIPPER_PRI_HDR_SIZE + secHdrSize + payloadSize;
    FW_ASSERT(totalSize <= sizeof(this->m_packetStorage), static_cast<FwAssertArgType>(totalSize));
    U8* const bytes = this->m_packetStorage;
    // Primary header: version 0, type/sec-hdr flags, APID 0x123, sequence flags 0b11, count 0x2A
    bytes[0] = static_cast<U8>((command ? CFS_TLM_STRIPPER_TYPE_FLAG : 0) |
                               (secHdr ? CFS_TLM_STRIPPER_SEC_HDR_FLAG : 0) | 0x01);
    bytes[1] = 0x23;
    bytes[2] = 0xC0;
    bytes[3] = 0x2A;
    // Packet data length field: bytes after the primary header, minus one
    const FwSizeType lengthToken = secHdrSize + payloadSize - 1;
    bytes[4] = static_cast<U8>(lengthToken >> 8);
    bytes[5] = static_cast<U8>(lengthToken & 0xFF);
    for (FwSizeType i = 0; i < secHdrSize + payloadSize; i++) {
        bytes[CFS_TLM_STRIPPER_PRI_HDR_SIZE + i] = static_cast<U8>(STest::Random::lowerUpper(0, 0xFF));
    }
    return Fw::Buffer(bytes, static_cast<Fw::Buffer::SizeType>(totalSize));
}

// ----------------------------------------------------------------------
// Tests
// ----------------------------------------------------------------------

void CfsTlmStripperTester ::testTelemetryStrip() {
    const FwSizeType payloadSize = 10;
    Fw::Buffer data = this->buildPacket(false, true, payloadSize);
    U8 original[sizeof(this->m_packetStorage)];
    (void)std::memcpy(original, data.getData(), data.getSize());

    ComCfg::FrameContext context;
    context.set_hasSecHdr(true);
    this->invoke_to_dataIn(0, data, context);

    ASSERT_from_dataOut_SIZE(1);
    Fw::Buffer out = this->fromPortHistory_dataOut->at(0).data;
    // In place: the output points into the same storage, past the removed header bytes
    ASSERT_EQ(out.getData(), &this->m_packetStorage[CFS_TLM_STRIPPER_SEC_HDR_SIZE]);
    ASSERT_EQ(out.getSize(), CFS_TLM_STRIPPER_PRI_HDR_SIZE + payloadSize);
    ASSERT_FALSE(this->fromPortHistory_dataOut->at(0).context.get_hasSecHdr());

    const U8* const header = out.getData();
    // Secondary header flag cleared; other streamId bits and sequence field preserved
    ASSERT_EQ(header[0], static_cast<U8>(original[0] & ~CFS_TLM_STRIPPER_SEC_HDR_FLAG));
    ASSERT_EQ(header[1], original[1]);
    ASSERT_EQ(header[2], original[2]);
    ASSERT_EQ(header[3], original[3]);
    // Length field reduced by the secondary header size
    const FwSizeType lengthToken =
        (static_cast<FwSizeType>(header[4]) << 8) | static_cast<FwSizeType>(header[5]);
    ASSERT_EQ(lengthToken, payloadSize - 1);
    // Payload preserved unchanged
    for (FwSizeType i = 0; i < payloadSize; i++) {
        ASSERT_EQ(header[CFS_TLM_STRIPPER_PRI_HDR_SIZE + i],
                  original[CFS_TLM_STRIPPER_PRI_HDR_SIZE + CFS_TLM_STRIPPER_SEC_HDR_SIZE + i]);
    }
    ASSERT_EVENTS_MalformedPacket_SIZE(0);
}

void CfsTlmStripperTester ::testCommandPassthrough() {
    const FwSizeType payloadSize = 8;
    Fw::Buffer data = this->buildPacket(true, true, payloadSize);
    U8 original[sizeof(this->m_packetStorage)];
    (void)std::memcpy(original, data.getData(), data.getSize());

    ComCfg::FrameContext context;
    context.set_hasSecHdr(true);
    this->invoke_to_dataIn(0, data, context);

    ASSERT_from_dataOut_SIZE(1);
    Fw::Buffer out = this->fromPortHistory_dataOut->at(0).data;
    ASSERT_EQ(out.getData(), this->m_packetStorage);
    ASSERT_EQ(out.getSize(), data.getSize());
    ASSERT_TRUE(this->fromPortHistory_dataOut->at(0).context.get_hasSecHdr());
    for (FwSizeType i = 0; i < out.getSize(); i++) {
        ASSERT_EQ(out.getData()[i], original[i]);
    }
}

void CfsTlmStripperTester ::testNoSecHdrPassthrough() {
    const FwSizeType payloadSize = 8;
    Fw::Buffer data = this->buildPacket(false, false, payloadSize);
    U8 original[sizeof(this->m_packetStorage)];
    (void)std::memcpy(original, data.getData(), data.getSize());

    ComCfg::FrameContext context;
    this->invoke_to_dataIn(0, data, context);

    ASSERT_from_dataOut_SIZE(1);
    Fw::Buffer out = this->fromPortHistory_dataOut->at(0).data;
    ASSERT_EQ(out.getData(), this->m_packetStorage);
    ASSERT_EQ(out.getSize(), data.getSize());
    for (FwSizeType i = 0; i < out.getSize(); i++) {
        ASSERT_EQ(out.getData()[i], original[i]);
    }
}

void CfsTlmStripperTester ::testMalformedPacket() {
    U8 shortData[CFS_TLM_STRIPPER_PRI_HDR_SIZE - 1] = {};
    Fw::Buffer data(shortData, sizeof(shortData));
    ComCfg::FrameContext context;
    this->invoke_to_dataIn(0, data, context);

    ASSERT_from_dataOut_SIZE(0);
    ASSERT_from_dataReturnOut_SIZE(1);
    ASSERT_EQ(this->fromPortHistory_dataReturnOut->at(0).data.getData(), shortData);
    ASSERT_EVENTS_MalformedPacket_SIZE(1);
    ASSERT_EVENTS_MalformedPacket(0, sizeof(shortData));
}

void CfsTlmStripperTester ::testDataReturnPassthrough() {
    U8 storage[8] = {};
    Fw::Buffer buffer(storage, sizeof(storage));
    ComCfg::FrameContext context;
    this->invoke_to_dataReturnIn(0, buffer, context);

    ASSERT_from_dataReturnOut_SIZE(1);
    ASSERT_EQ(this->fromPortHistory_dataReturnOut->at(0).data.getData(), storage);
}

void CfsTlmStripperTester ::testComStatusPassthrough() {
    Fw::Success status = Fw::Success::SUCCESS;
    this->invoke_to_comStatusIn(0, status);
    ASSERT_from_comStatusOut_SIZE(1);
    ASSERT_EQ(this->fromPortHistory_comStatusOut->at(0).condition, status);
    this->clearHistory();
    status = Fw::Success::FAILURE;
    this->invoke_to_comStatusIn(0, status);
    ASSERT_from_comStatusOut_SIZE(1);
    ASSERT_EQ(this->fromPortHistory_comStatusOut->at(0).condition, status);
}

}  // namespace FPrimeCfs
