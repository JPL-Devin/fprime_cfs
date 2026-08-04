// ======================================================================
// \title  CfsCmdRouterTester.cpp
// \brief  cpp file for CfsCmdRouter component test harness implementation class
// ======================================================================

#include "CfsCmdRouterTester.hpp"
#include <STest/Pick/Pick.hpp>
#include <cstring>

namespace FPrimeCfs {

// Function codes matching the static routing table configured in CfsCmdRouterCfg.fpp
static const U8 COM_FUNCTION_CODE = 0;
static const U8 BUFFER_FUNCTION_CODE = 1;
static const U8 UNKNOWN_FUNCTION_CODE = 2;

static const ComCfg::Apid::T TEST_APID = ComCfg::Apid::FW_PACKET_HAND;

// ----------------------------------------------------------------------
// Construction and destruction
// ----------------------------------------------------------------------

CfsCmdRouterTester ::CfsCmdRouterTester(bool connectOutputs)
    : CfsCmdRouterGTestBase("CfsCmdRouterTester", CfsCmdRouterTester::MAX_HISTORY_SIZE), component("CfsCmdRouter") {
    this->initComponents();
    if (connectOutputs) {
        this->connectPorts();
    } else {
        // Connect only the input ports and the data return path, leaving all
        // route output ports disconnected
        this->connect_to_dataIn(0, this->component.get_dataIn_InputPort(0));
        this->connect_to_bufferReturnIn(0, this->component.get_bufferReturnIn_InputPort(0));
        this->component.set_dataReturnOut_OutputPort(0, this->get_from_dataReturnOut(0));
        this->component.set_timeCaller_OutputPort(0, this->get_from_timeCaller(0));
        this->component.set_logOut_OutputPort(0, this->get_from_logOut(0));
        this->component.set_logTextOut_OutputPort(0, this->get_from_logTextOut(0));
    }
}

CfsCmdRouterTester ::~CfsCmdRouterTester() {}

// ----------------------------------------------------------------------
// Helpers
// ----------------------------------------------------------------------

void CfsCmdRouterTester ::setValidChecksum(ComCfg::Apid::T apid, U16 sequenceCount, U8* bytes, FwSizeType size) {
    // Set the checksum byte so the XOR of every packet byte (reconstructed primary
    // header plus data) with 0xFF equals zero.
    bytes[1] = 0;
    const FwSizeType lengthToken = size - 1;
    U8 checksum = 0xFF;
    checksum ^= static_cast<U8>(0x18 | ((static_cast<U16>(apid) >> 8) & 0x07));
    checksum ^= static_cast<U8>(static_cast<U16>(apid) & 0xFF);
    checksum ^= static_cast<U8>(0xC0 | ((sequenceCount >> 8) & 0x3F));
    checksum ^= static_cast<U8>(sequenceCount & 0xFF);
    checksum ^= static_cast<U8>((lengthToken >> 8) & 0xFF);
    checksum ^= static_cast<U8>(lengthToken & 0xFF);
    for (FwSizeType i = 0; i < size; i++) {
        checksum ^= bytes[i];
    }
    bytes[1] = checksum;
}

void CfsCmdRouterTester ::sendData(ComCfg::Apid::T apid,
                                   bool hasSecHdr,
                                   U8* bytes,
                                   FwSizeType size,
                                   U16 sequenceCount) {
    Fw::Buffer buffer(bytes, size);
    ComCfg::FrameContext context;
    context.set_apid(apid);
    context.set_hasSecHdr(hasSecHdr);
    context.set_sequenceCount(sequenceCount);
    this->invoke_to_dataIn(0, buffer, context);
}

// ----------------------------------------------------------------------
// Tests
// ----------------------------------------------------------------------

void CfsCmdRouterTester ::testRouteCom() {
    U8 bytes[8] = {COM_FUNCTION_CODE, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06};
    CfsCmdRouterTester::setValidChecksum(TEST_APID, 0, bytes, sizeof(bytes));
    this->sendData(TEST_APID, true, bytes, sizeof(bytes));
    // Message copied out without the 2-byte secondary header
    ASSERT_from_comOut_SIZE(1);
    const Fw::ComBuffer& com = this->fromPortHistory_comOut->at(0).data;
    ASSERT_EQ(com.getSize(), sizeof(bytes) - CFS_CMD_ROUTER_SEC_HDR_SIZE);
    ASSERT_EQ(std::memcmp(com.getBuffAddr(), &bytes[CFS_CMD_ROUTER_SEC_HDR_SIZE],
                          sizeof(bytes) - CFS_CMD_ROUTER_SEC_HDR_SIZE),
              0);
    // Buffer returned immediately
    ASSERT_from_dataReturnOut_SIZE(1);
    ASSERT_EQ(this->fromPortHistory_dataReturnOut->at(0).data.getData(), bytes);
    // The buffer is returned with the context it was received with
    ASSERT_EQ(this->fromPortHistory_dataReturnOut->at(0).context.get_apid(), TEST_APID);
    ASSERT_EVENTS_SIZE(0);
}

void CfsCmdRouterTester ::testRouteComTooLarge() {
    U8 bytes[FW_COM_BUFFER_MAX_SIZE + CFS_CMD_ROUTER_SEC_HDR_SIZE + 1] = {COM_FUNCTION_CODE};
    CfsCmdRouterTester::setValidChecksum(TEST_APID, 0, bytes, sizeof(bytes));
    this->sendData(TEST_APID, true, bytes, sizeof(bytes));
    ASSERT_from_comOut_SIZE(0);
    ASSERT_EVENTS_SerializationError_SIZE(1);
    // Buffer still returned
    ASSERT_from_dataReturnOut_SIZE(1);
}

void CfsCmdRouterTester ::testRouteBuffer() {
    U8 bytes[6] = {BUFFER_FUNCTION_CODE, 0x00, 0x11, 0x22, 0x33, 0x44};
    CfsCmdRouterTester::setValidChecksum(TEST_APID, 0, bytes, sizeof(bytes));
    this->sendData(TEST_APID, true, bytes, sizeof(bytes));
    ASSERT_from_bufferOut_SIZE(1);
    ASSERT_EQ(this->fromPortHistory_bufferOut->at(0).functionCode, BUFFER_FUNCTION_CODE);
    Fw::Buffer payload = this->fromPortHistory_bufferOut->at(0).data;
    ASSERT_EQ(payload.getData(), &bytes[CFS_CMD_ROUTER_SEC_HDR_SIZE]);
    ASSERT_EQ(payload.getSize(), sizeof(bytes) - CFS_CMD_ROUTER_SEC_HDR_SIZE);
    // Ownership not yet returned
    ASSERT_from_dataReturnOut_SIZE(0);
    // Return the buffer; it is forwarded to dataReturnOut
    this->invoke_to_bufferReturnIn(0, payload);
    ASSERT_from_dataReturnOut_SIZE(1);
    // The buffer is returned with the context it was received with
    ASSERT_EQ(this->fromPortHistory_dataReturnOut->at(0).context.get_apid(), TEST_APID);
    ASSERT_EVENTS_SIZE(0);
}

void CfsCmdRouterTester ::testRouteUnknown() {
    U8 bytes[4] = {UNKNOWN_FUNCTION_CODE, 0x00, 0x03, 0x04};
    CfsCmdRouterTester::setValidChecksum(TEST_APID, 0, bytes, sizeof(bytes));
    this->sendData(TEST_APID, true, bytes, sizeof(bytes));
    ASSERT_from_unknownDataOut_SIZE(1);
    ASSERT_EQ(this->fromPortHistory_unknownDataOut->at(0).context.get_apid(), TEST_APID);
    Fw::Buffer buffer = this->fromPortHistory_unknownDataOut->at(0).data;
    ASSERT_EQ(buffer.getData(), bytes);
    ASSERT_from_dataReturnOut_SIZE(0);
    this->invoke_to_bufferReturnIn(0, buffer);
    ASSERT_from_dataReturnOut_SIZE(1);
    // The buffer is returned with the context it was received with
    ASSERT_EQ(this->fromPortHistory_dataReturnOut->at(0).context.get_apid(), TEST_APID);
    ASSERT_EVENTS_SIZE(0);
}

void CfsCmdRouterTester ::testBadChecksum() {
    U8 bytes[6] = {COM_FUNCTION_CODE, 0x00, 0x11, 0x22, 0x33, 0x44};
    CfsCmdRouterTester::setValidChecksum(TEST_APID, 0, bytes, sizeof(bytes));
    bytes[1] ^= 0xA5;  // Corrupt the checksum
    this->sendData(TEST_APID, true, bytes, sizeof(bytes));
    // Dropped: nothing routed, buffer returned, warning emitted
    ASSERT_from_comOut_SIZE(0);
    ASSERT_from_bufferOut_SIZE(0);
    ASSERT_from_unknownDataOut_SIZE(0);
    ASSERT_from_dataReturnOut_SIZE(1);
    ASSERT_EVENTS_BadChecksum_SIZE(1);
    ASSERT_EVENTS_BadChecksum(0, static_cast<U16>(TEST_APID), COM_FUNCTION_CODE, 0xA5);
    // A message valid under a different sequence count fails when the context differs
    CfsCmdRouterTester::setValidChecksum(TEST_APID, 5, bytes, sizeof(bytes));
    this->sendData(TEST_APID, true, bytes, sizeof(bytes), 6);
    ASSERT_EVENTS_BadChecksum_SIZE(2);
    ASSERT_from_dataReturnOut_SIZE(2);
}

void CfsCmdRouterTester ::testMissingSecondaryHeader() {
    U8 bytes[6] = {COM_FUNCTION_CODE, 0x00, 0x11, 0x22, 0x33, 0x44};
    this->sendData(TEST_APID, false, bytes, sizeof(bytes));
    ASSERT_from_comOut_SIZE(0);
    ASSERT_from_unknownDataOut_SIZE(1);
    ASSERT_EVENTS_MissingSecondaryHeader_SIZE(1);
    Fw::Buffer buffer = this->fromPortHistory_unknownDataOut->at(0).data;
    this->invoke_to_bufferReturnIn(0, buffer);
    ASSERT_from_dataReturnOut_SIZE(1);
}

void CfsCmdRouterTester ::testShortSecondaryHeader() {
    U8 bytes[1] = {COM_FUNCTION_CODE};
    this->sendData(TEST_APID, true, bytes, sizeof(bytes));
    ASSERT_from_comOut_SIZE(0);
    ASSERT_from_unknownDataOut_SIZE(1);
    ASSERT_EVENTS_MissingSecondaryHeader_SIZE(1);
}

void CfsCmdRouterTester ::testDisconnectedOutputs() {
    // Constructed with connectOutputs == false: all route outputs disconnected
    U8 bytes[8] = {COM_FUNCTION_CODE, 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66};
    CfsCmdRouterTester::setValidChecksum(TEST_APID, 0, bytes, sizeof(bytes));
    this->sendData(TEST_APID, true, bytes, sizeof(bytes));
    ASSERT_from_dataReturnOut_SIZE(1);
    bytes[0] = BUFFER_FUNCTION_CODE;
    CfsCmdRouterTester::setValidChecksum(TEST_APID, 0, bytes, sizeof(bytes));
    this->sendData(TEST_APID, true, bytes, sizeof(bytes));
    ASSERT_from_dataReturnOut_SIZE(2);
    bytes[0] = UNKNOWN_FUNCTION_CODE;
    CfsCmdRouterTester::setValidChecksum(TEST_APID, 0, bytes, sizeof(bytes));
    this->sendData(TEST_APID, true, bytes, sizeof(bytes));
    ASSERT_from_dataReturnOut_SIZE(3);
}

void CfsCmdRouterTester ::testCommandResponseNoop() {
    this->invoke_to_cmdResponseIn(0, 0x123, 7, Fw::CmdResponse::OK);
    ASSERT_from_dataReturnOut_SIZE(0);
    ASSERT_from_comOut_SIZE(0);
    ASSERT_EVENTS_SIZE(0);
}

void CfsCmdRouterTester ::testRandomized() {
    U32 returned = 0;
    for (U32 i = 0; i < 1000; i++) {
        U8 bytes[32];
        for (FwSizeType j = 0; j < sizeof(bytes); j++) {
            bytes[j] = static_cast<U8>(STest::Pick::lowerUpper(0, 0xFF));
        }
        const FwSizeType size =
            static_cast<FwSizeType>(STest::Pick::lowerUpper(CFS_CMD_ROUTER_SEC_HDR_SIZE, sizeof(bytes)));
        static const U8 FUNCTION_CODES[3] = {COM_FUNCTION_CODE, BUFFER_FUNCTION_CODE, UNKNOWN_FUNCTION_CODE};
        bytes[0] = FUNCTION_CODES[STest::Pick::lowerUpper(0, 2)];
        const bool hasSecHdr = (STest::Pick::lowerUpper(0, 1) == 1);
        const bool validChecksum = (STest::Pick::lowerUpper(0, 1) == 1);
        if (validChecksum) {
            CfsCmdRouterTester::setValidChecksum(TEST_APID, 0, bytes, size);
        }
        this->sendData(TEST_APID, hasSecHdr, bytes, size);
        // Return any buffer handed out on a pass-through route
        if (this->fromPortHistory_bufferOut->size() > 0) {
            Fw::Buffer buffer = this->fromPortHistory_bufferOut->at(0).data;
            this->invoke_to_bufferReturnIn(0, buffer);
        } else if (this->fromPortHistory_unknownDataOut->size() > 0) {
            Fw::Buffer buffer = this->fromPortHistory_unknownDataOut->at(0).data;
            this->invoke_to_bufferReturnIn(0, buffer);
        }
        // Every message results in exactly one buffer return
        ASSERT_from_dataReturnOut_SIZE(1);
        returned++;
        this->clearHistory();
    }
    ASSERT_EQ(returned, 1000u);
}

}  // namespace FPrimeCfs
