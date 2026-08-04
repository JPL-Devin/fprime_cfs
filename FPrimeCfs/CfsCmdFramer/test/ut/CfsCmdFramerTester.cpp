// ======================================================================
// \title  CfsCmdFramerTester.cpp
// \brief  cpp file for CfsCmdFramer component test harness implementation class
// ======================================================================

#include "CfsCmdFramerTester.hpp"
#include "STest/Random/Random.hpp"
#include "Svc/Ccsds/Types/FppConstantsAc.hpp"

namespace FPrimeCfs {

// ----------------------------------------------------------------------
// Construction and destruction
// ----------------------------------------------------------------------

CfsCmdFramerTester ::CfsCmdFramerTester()
    : CfsCmdFramerGTestBase("CfsCmdFramerTester", CfsCmdFramerTester::MAX_HISTORY_SIZE),
      component("CfsCmdFramer") {
    this->initComponents();
    this->connectPorts();
}

CfsCmdFramerTester ::~CfsCmdFramerTester() {}

// ----------------------------------------------------------------------
// Tests
// ----------------------------------------------------------------------

void CfsCmdFramerTester ::testNominalFraming() {
    U8 payload[16];
    for (U32 i = 0; i < sizeof(payload); ++i) {
        payload[i] = static_cast<U8>(STest::Random::lowerUpper(0, 0xFF));
    }
    Fw::Buffer data(payload, sizeof(payload));
    const U8 functionCode = static_cast<U8>(STest::Random::lowerUpper(0, 0x7F));
    ComCfg::FrameContext context;
    context.set_apid(ComCfg::Apid::FW_PACKET_COMMAND);
    context.set_functionCode(functionCode);

    this->invoke_to_dataIn(0, data, context);

    // Check dataOut: secondary header prepended
    ASSERT_from_dataOut_SIZE(1);
    Fw::Buffer outBuffer = this->fromPortHistory_dataOut->at(0).data;
    ASSERT_EQ(outBuffer.getSize(), sizeof(payload) + CFS_CMD_FRAMER_SEC_HDR_SIZE);
    ASSERT_EQ(outBuffer.getData()[0], functionCode);
    for (U32 i = 0; i < sizeof(payload); ++i) {
        ASSERT_EQ(outBuffer.getData()[CFS_CMD_FRAMER_SEC_HDR_SIZE + i], payload[i]);
    }

    // Check that the secondary header flag was set in the outgoing context
    ASSERT_TRUE(this->fromPortHistory_dataOut->at(0).context.get_hasSecHdr());

    // Check that dataReturnOut is called for the original buffer with the original context
    ASSERT_from_dataReturnOut_SIZE(1);
    ASSERT_EQ(this->fromPortHistory_dataReturnOut->at(0).data.getData(), payload);
    ASSERT_FALSE(this->fromPortHistory_dataReturnOut->at(0).context.get_hasSecHdr());
}

void CfsCmdFramerTester ::testChecksum() {
    U8 payload[8];
    for (U32 i = 0; i < sizeof(payload); ++i) {
        payload[i] = static_cast<U8>(STest::Random::lowerUpper(0, 0xFF));
    }
    Fw::Buffer data(payload, sizeof(payload));
    const U16 apid = static_cast<U16>(ComCfg::Apid::FW_PACKET_COMMAND);
    const U16 seqCount = static_cast<U16>(STest::Random::lowerUpper(0, 0x3FFF));
    ComCfg::FrameContext context;
    context.set_apid(ComCfg::Apid::FW_PACKET_COMMAND);
    context.set_sequenceCount(seqCount);
    context.set_functionCode(static_cast<U8>(STest::Random::lowerUpper(0, 0x7F)));

    this->invoke_to_dataIn(0, data, context);

    ASSERT_from_dataOut_SIZE(1);
    Fw::Buffer outBuffer = this->fromPortHistory_dataOut->at(0).data;

    // Reconstruct the predicted complete packet: primary header (command type,
    // secondary header flag, unsegmented, context sequence count) + output
    const FwSizeType length = outBuffer.getSize() - 1;
    const U16 packetIdentification = static_cast<U16>(apid | 0x0800 | 0x1000);
    const U16 packetSequenceControl = static_cast<U16>(0xC000 | seqCount);
    U8 checksum = 0xFF;
    checksum ^= static_cast<U8>(packetIdentification >> 8);
    checksum ^= static_cast<U8>(packetIdentification & 0xFF);
    checksum ^= static_cast<U8>(packetSequenceControl >> 8);
    checksum ^= static_cast<U8>(packetSequenceControl & 0xFF);
    checksum ^= static_cast<U8>((length >> 8) & 0xFF);
    checksum ^= static_cast<U8>(length & 0xFF);
    for (FwSizeType i = 0; i < outBuffer.getSize(); i++) {
        checksum ^= outBuffer.getData()[i];
    }
    // XOR of the full predicted packet (seeded 0xFF) must equal zero
    ASSERT_EQ(checksum, 0);
}

void CfsCmdFramerTester ::testAllocationFailure() {
    U8 payload[16] = {};
    Fw::Buffer data(payload, sizeof(payload));
    ComCfg::FrameContext context;

    this->m_useUndersizedAlloc = true;
    this->invoke_to_dataIn(0, data, context);
    this->m_useUndersizedAlloc = false;

    ASSERT_from_dataOut_SIZE(0);
    ASSERT_from_dataReturnOut_SIZE(1);
    ASSERT_EQ(this->fromPortHistory_dataReturnOut->at(0).data.getData(), payload);
    ASSERT_EVENTS_AllocationFailed_SIZE(1);
    // The undersized (but non-empty) allocation must be deallocated
    ASSERT_from_bufferDeallocate_SIZE(1);
}

void CfsCmdFramerTester ::testComStatusPassthrough() {
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

void CfsCmdFramerTester ::testDataReturnPassthrough() {
    U8 data[8] = {};
    Fw::Buffer buffer(data, sizeof(data));
    ComCfg::FrameContext context;
    this->invoke_to_dataReturnIn(0, buffer, context);
    ASSERT_from_bufferDeallocate_SIZE(1);
    ASSERT_EQ(this->fromPortHistory_bufferDeallocate->at(0).fwBuffer.getData(), data);
}

// ----------------------------------------------------------------------
// Output port handler overrides
// ----------------------------------------------------------------------

Fw::Buffer CfsCmdFramerTester ::from_bufferAllocate_handler(FwIndexType portNum, FwSizeType size) {
    const FwSizeType allocation = this->m_useUndersizedAlloc ? (size - 1) : size;
    FW_ASSERT(allocation <= sizeof(this->m_allocStorage), static_cast<FwAssertArgType>(allocation));
    return Fw::Buffer(this->m_allocStorage, allocation);
}

}  // namespace FPrimeCfs
