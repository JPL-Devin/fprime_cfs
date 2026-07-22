// ======================================================================
// \title  CfsRouterTester.cpp
// \brief  cpp file for CfsRouter component test harness implementation class
// ======================================================================

#include "CfsRouterTester.hpp"
#include <STest/Pick/Pick.hpp>
#include <cstring>

namespace FPrimeCfs {

// The standard test routing table
static const CfsRouteEntry TEST_TABLE[] = {
    {ComCfg::Apid::FW_PACKET_COMMAND, CfsRouteType::FPRIME_COMMAND, 0},
    {ComCfg::Apid::FW_PACKET_TELEM, CfsRouteType::CFS_COMMAND, 1},
    {ComCfg::Apid::FW_PACKET_LOG, CfsRouteType::CFS_TELEMETRY, 2},
};
static const FwSizeType TEST_TABLE_ENTRIES = FW_NUM_ARRAY_ELEMENTS(TEST_TABLE);

// ----------------------------------------------------------------------
// Construction and destruction
// ----------------------------------------------------------------------

CfsRouterTester ::CfsRouterTester(bool connectOutputs)
    : CfsRouterGTestBase("CfsRouterTester", CfsRouterTester::MAX_HISTORY_SIZE), component("CfsRouter") {
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

CfsRouterTester ::~CfsRouterTester() {}

// ----------------------------------------------------------------------
// Helpers
// ----------------------------------------------------------------------

void CfsRouterTester ::configureTable() {
    this->component.configure(TEST_TABLE, TEST_TABLE_ENTRIES);
}

void CfsRouterTester ::sendData(ComCfg::Apid::T apid, bool hasSecHdr, U8* bytes, FwSizeType size) {
    Fw::Buffer buffer(bytes, size);
    ComCfg::FrameContext context;
    context.set_apid(apid);
    context.set_hasSecHdr(hasSecHdr);
    this->invoke_to_dataIn(0, buffer, context);
}

// ----------------------------------------------------------------------
// Tests
// ----------------------------------------------------------------------

void CfsRouterTester ::testRouteFprimeCommand() {
    this->configureTable();
    U8 bytes[8] = {0xDE, 0xAD, 0xBE, 0xEF, 0x01, 0x02, 0x03, 0x04};
    this->sendData(ComCfg::Apid::FW_PACKET_COMMAND, false, bytes, sizeof(bytes));
    // Command copied out on the configured index
    ASSERT_from_commandOut_SIZE(1);
    const Fw::ComBuffer& com = this->fromPortHistory_commandOut->at(0).data;
    ASSERT_EQ(com.getBuffLength(), sizeof(bytes));
    ASSERT_EQ(std::memcmp(com.getBuffAddr(), bytes, sizeof(bytes)), 0);
    // Buffer returned immediately
    ASSERT_from_dataReturnOut_SIZE(1);
    ASSERT_EQ(this->fromPortHistory_dataReturnOut->at(0).data.getData(), bytes);
    ASSERT_EVENTS_SIZE(0);
}

void CfsRouterTester ::testRouteFprimeCommandSecHdr() {
    this->configureTable();
    U8 bytes[8] = {0xAA, 0x55, 0xBE, 0xEF, 0x01, 0x02, 0x03, 0x04};
    this->sendData(ComCfg::Apid::FW_PACKET_COMMAND, true, bytes, sizeof(bytes));
    // The 2-byte cFS command secondary header is excluded from the copy
    ASSERT_from_commandOut_SIZE(1);
    const Fw::ComBuffer& com = this->fromPortHistory_commandOut->at(0).data;
    ASSERT_EQ(com.getBuffLength(), sizeof(bytes) - CFS_ROUTER_CMD_SEC_HDR_SIZE);
    ASSERT_EQ(std::memcmp(com.getBuffAddr(), &bytes[CFS_ROUTER_CMD_SEC_HDR_SIZE],
                          sizeof(bytes) - CFS_ROUTER_CMD_SEC_HDR_SIZE),
              0);
    ASSERT_from_dataReturnOut_SIZE(1);
    ASSERT_EVENTS_SIZE(0);
}

void CfsRouterTester ::testRouteFprimeCommandTooLarge() {
    this->configureTable();
    U8 bytes[FW_COM_BUFFER_MAX_SIZE + 1] = {};
    this->sendData(ComCfg::Apid::FW_PACKET_COMMAND, false, bytes, sizeof(bytes));
    ASSERT_from_commandOut_SIZE(0);
    ASSERT_EVENTS_SerializationError_SIZE(1);
    // Buffer still returned
    ASSERT_from_dataReturnOut_SIZE(1);
}

void CfsRouterTester ::testRouteCfsCommand() {
    this->configureTable();
    U8 bytes[6] = {0x2A, 0xFF, 0x11, 0x22, 0x33, 0x44};  // fn code 0x2A, checksum, payload
    this->sendData(ComCfg::Apid::FW_PACKET_TELEM, true, bytes, sizeof(bytes));
    ASSERT_from_cfsCommandOut_SIZE(1);
    ASSERT_EQ(this->fromPortHistory_cfsCommandOut->at(0).functionCode, 0x2A);
    Fw::Buffer payload = this->fromPortHistory_cfsCommandOut->at(0).data;
    ASSERT_EQ(payload.getData(), &bytes[CFS_ROUTER_CMD_SEC_HDR_SIZE]);
    ASSERT_EQ(payload.getSize(), sizeof(bytes) - CFS_ROUTER_CMD_SEC_HDR_SIZE);
    // Ownership not yet returned
    ASSERT_from_dataReturnOut_SIZE(0);
    // Return the buffer; it is forwarded to dataReturnOut
    this->invoke_to_bufferReturnIn(0, payload);
    ASSERT_from_dataReturnOut_SIZE(1);
    ASSERT_EVENTS_SIZE(0);
}

void CfsRouterTester ::testRouteCfsTelemetry() {
    this->configureTable();
    // Big-endian time: seconds = 0x01020304, subseconds16 = 0xABCD; then payload
    U8 bytes[9] = {0x01, 0x02, 0x03, 0x04, 0xAB, 0xCD, 0x77, 0x88, 0x99};
    this->sendData(ComCfg::Apid::FW_PACKET_LOG, true, bytes, sizeof(bytes));
    ASSERT_from_cfsTelemetryOut_SIZE(1);
    const CfsTime& time = this->fromPortHistory_cfsTelemetryOut->at(0).sysTime;
    ASSERT_EQ(time.get_seconds(), 0x01020304u);
    ASSERT_EQ(time.get_subseconds(), 0xABCD0000u);
    Fw::Buffer payload = this->fromPortHistory_cfsTelemetryOut->at(0).data;
    ASSERT_EQ(payload.getData(), &bytes[CFS_ROUTER_TLM_SEC_HDR_SIZE]);
    ASSERT_EQ(payload.getSize(), sizeof(bytes) - CFS_ROUTER_TLM_SEC_HDR_SIZE);
    ASSERT_from_dataReturnOut_SIZE(0);
    this->invoke_to_bufferReturnIn(0, payload);
    ASSERT_from_dataReturnOut_SIZE(1);
    ASSERT_EVENTS_SIZE(0);
}

void CfsRouterTester ::testRouteUnknown() {
    this->configureTable();
    U8 bytes[4] = {0x01, 0x02, 0x03, 0x04};
    this->sendData(ComCfg::Apid::FW_PACKET_FILE, false, bytes, sizeof(bytes));
    ASSERT_from_unknownDataOut_SIZE(1);
    ASSERT_EQ(this->fromPortHistory_unknownDataOut->at(0).context.get_apid(), ComCfg::Apid::FW_PACKET_FILE);
    Fw::Buffer buffer = this->fromPortHistory_unknownDataOut->at(0).data;
    ASSERT_EQ(buffer.getData(), bytes);
    ASSERT_from_dataReturnOut_SIZE(0);
    this->invoke_to_bufferReturnIn(0, buffer);
    ASSERT_from_dataReturnOut_SIZE(1);
    ASSERT_EVENTS_SIZE(0);
}

void CfsRouterTester ::testMissingSecondaryHeader() {
    this->configureTable();
    U8 bytes[6] = {0x2A, 0xFF, 0x11, 0x22, 0x33, 0x44};
    // cFS command APID but no secondary header flag
    this->sendData(ComCfg::Apid::FW_PACKET_TELEM, false, bytes, sizeof(bytes));
    ASSERT_from_cfsCommandOut_SIZE(0);
    ASSERT_from_unknownDataOut_SIZE(1);
    ASSERT_EVENTS_MissingSecondaryHeader_SIZE(1);
    // cFS telemetry APID but no secondary header flag
    this->sendData(ComCfg::Apid::FW_PACKET_LOG, false, bytes, sizeof(bytes));
    ASSERT_from_cfsTelemetryOut_SIZE(0);
    ASSERT_from_unknownDataOut_SIZE(2);
    ASSERT_EVENTS_MissingSecondaryHeader_SIZE(2);
}

void CfsRouterTester ::testShortSecondaryHeader() {
    this->configureTable();
    U8 bytes[1] = {0x2A};
    this->sendData(ComCfg::Apid::FW_PACKET_TELEM, true, bytes, sizeof(bytes));
    ASSERT_from_cfsCommandOut_SIZE(0);
    ASSERT_from_unknownDataOut_SIZE(1);
    ASSERT_EVENTS_MissingSecondaryHeader_SIZE(1);
    U8 tlmBytes[CFS_ROUTER_TLM_SEC_HDR_SIZE - 1] = {};
    this->sendData(ComCfg::Apid::FW_PACKET_LOG, true, tlmBytes, sizeof(tlmBytes));
    ASSERT_from_cfsTelemetryOut_SIZE(0);
    ASSERT_from_unknownDataOut_SIZE(2);
    ASSERT_EVENTS_MissingSecondaryHeader_SIZE(2);
}

void CfsRouterTester ::testDisconnectedOutputs() {
    // Constructed with connectOutputs == false: all route outputs disconnected
    this->configureTable();
    U8 bytes[8] = {0x2A, 0xFF, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66};
    this->sendData(ComCfg::Apid::FW_PACKET_COMMAND, false, bytes, sizeof(bytes));
    ASSERT_from_dataReturnOut_SIZE(1);
    this->sendData(ComCfg::Apid::FW_PACKET_TELEM, true, bytes, sizeof(bytes));
    ASSERT_from_dataReturnOut_SIZE(2);
    this->sendData(ComCfg::Apid::FW_PACKET_LOG, true, bytes, sizeof(bytes));
    ASSERT_from_dataReturnOut_SIZE(3);
    this->sendData(ComCfg::Apid::FW_PACKET_FILE, false, bytes, sizeof(bytes));
    ASSERT_from_dataReturnOut_SIZE(4);
}

void CfsRouterTester ::testCommandResponseNoop() {
    this->configureTable();
    this->invoke_to_cmdResponseIn(0, 0x123, 7, Fw::CmdResponse::OK);
    ASSERT_from_dataReturnOut_SIZE(0);
    ASSERT_from_commandOut_SIZE(0);
    ASSERT_EVENTS_SIZE(0);
}

void CfsRouterTester ::testRandomized() {
    this->configureTable();
    U32 returned = 0;
    for (U32 i = 0; i < 1000; i++) {
        U8 bytes[32];
        for (FwSizeType j = 0; j < sizeof(bytes); j++) {
            bytes[j] = static_cast<U8>(STest::Pick::lowerUpper(0, 0xFF));
        }
        const FwSizeType size = static_cast<FwSizeType>(
            STest::Pick::lowerUpper(CFS_ROUTER_TLM_SEC_HDR_SIZE, sizeof(bytes)));
        const U32 pick = STest::Pick::lowerUpper(0, 3);
        static const ComCfg::Apid::T APIDS[4] = {ComCfg::Apid::FW_PACKET_COMMAND, ComCfg::Apid::FW_PACKET_TELEM,
                                                 ComCfg::Apid::FW_PACKET_LOG, ComCfg::Apid::FW_PACKET_FILE};
        const bool hasSecHdr = (STest::Pick::lowerUpper(0, 1) == 1);
        this->sendData(APIDS[pick], hasSecHdr, bytes, size);
        // Return any buffer handed out on a pass-through route
        if (this->fromPortHistory_cfsCommandOut->size() > 0) {
            Fw::Buffer buffer = this->fromPortHistory_cfsCommandOut->at(0).data;
            this->invoke_to_bufferReturnIn(0, buffer);
        } else if (this->fromPortHistory_cfsTelemetryOut->size() > 0) {
            Fw::Buffer buffer = this->fromPortHistory_cfsTelemetryOut->at(0).data;
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
