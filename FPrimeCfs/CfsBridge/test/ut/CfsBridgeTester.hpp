// ======================================================================
// \title  CfsBridgeTester.hpp
// \brief  hpp file for CfsBridge component test harness implementation class
// ======================================================================

#ifndef FPrimeCfs_CfsBridgeTester_HPP
#define FPrimeCfs_CfsBridgeTester_HPP

#include "CfsBridgeGTestBase.hpp"
#include "FPrimeCfs/CfsBridge/CfsBridge.hpp"
#include "CfeStubs.hpp"

namespace FPrimeCfs {

class CfsBridgeTester final : public CfsBridgeGTestBase {
  public:
    // Maximum size of histories storing events, telemetry, and port outputs
    static const U32 MAX_HISTORY_SIZE = 100;

    // Queue depth supplied to the component instance under test
    static const FwSizeType TEST_INSTANCE_QUEUE_DEPTH = 100;

    // Instance ID supplied to the component instance under test
    static const FwEnumStoreType TEST_INSTANCE_ID = 0;

    //! Construct object CfsBridgeTester
    CfsBridgeTester();

    //! Destroy object CfsBridgeTester
    ~CfsBridgeTester();

    // ----------------------------------------------------------------------
    // Tests: configure/subscribe
    // ----------------------------------------------------------------------

    //! Configure creates the software bus pipe with the supplied settings
    void testConfigure();

    //! Configure returns the software bus error on pipe creation failure
    void testConfigureFailure();

    //! Subscribe maps APIDs to the expected command/telemetry message ids
    void testSubscribe();

    //! Subscribe returns the software bus error on subscription failure
    void testSubscribeFailure();

    // ----------------------------------------------------------------------
    // Tests: transmit (dataIn -> software bus)
    // ----------------------------------------------------------------------

    //! A single complete space packet is transmitted as one software bus message
    void testTransmitSinglePacket();

    //! Multiple concatenated space packets are each transmitted as their own message
    void testTransmitMultiplePackets();

    //! Buffer ownership is returned and com status emitted on transmit failure
    void testTransmitFailure();

    //! A packet whose length field exceeds the available data is dropped
    void testTransmitTruncated();

    //! Residual bytes too small to form a primary header are dropped
    void testTransmitResidual();

    // ----------------------------------------------------------------------
    // Tests: receive (software bus -> dataOut)
    // ----------------------------------------------------------------------

    //! Received software bus messages are sent whole out dataOut with a default context
    void testReceive();

    //! process() emits a single preroll com status once subscribed
    void testPreroll();

    //! Empty software bus polls produce no output
    void testReceiveNoMessage();

    //! Software bus receive errors produce no output
    void testReceiveError();

    //! Messages whose size cannot be read are dropped
    void testReceiveGetSizeFailure();

    //! Flow control gates deframed messages on comStatusIn signals
    void testFlowControl();

    //! dataReturnIn accepts returned buffers without action
    void testDataReturn();

    //! Randomized sequence of uplink/downlink/flow-control operations
    void testRandomized();

  private:
    // ----------------------------------------------------------------------
    // Helpers
    // ----------------------------------------------------------------------

    //! Configure and subscribe the component to the given apid
    void configureAndSubscribe(ComCfg::Apid::T apid, bool paused = false);

    //! Build a CCSDS space packet with the given stream identifier and payload; returns the packet size
    FwSizeType makePacket(U8* dest, U16 streamIdValue, const U8* payload, FwSizeType payloadSize);

    //! Fill a byte array with random data
    void fillRandom(U8* data, FwSizeType size);

    //! Send a buffer through dataIn (dispatching the queued message) and assert
    //! buffer return and com status emission
    void sendDataIn(Fw::Buffer& buffer, const ComCfg::FrameContext& context);

    //! Queue a software bus message and poll it through via process()
    void receiveMessage(ComCfg::Apid::T apid, const U8* payload, FwSizeType size);

    //! Assert that a transmit call matches the given expectations
    void assertTransmitted(U32 index,
                           CFE_SB_MsgId_Atom_t expectedMsgId,
                           const U8* expectedPayload,
                           FwSizeType expectedPayloadSize);

    void connectPorts();    //!< Connects all ports for the component under test (auto-generated)
    void initComponents();  //!< Initializes the component under test (auto-generated)

    //! The component under test
    CfsBridge component;
};

}  // namespace FPrimeCfs

#endif
