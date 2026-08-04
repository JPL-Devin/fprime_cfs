// ======================================================================
// \title  CfsTlmFramerTester.hpp
// \brief  hpp file for CfsTlmFramer component test harness implementation class
// ======================================================================

#ifndef FPrimeCfs_CfsTlmFramerTester_HPP
#define FPrimeCfs_CfsTlmFramerTester_HPP

#include "CfsTlmFramerGTestBase.hpp"
#include "FPrimeCfs/CfsTlmFramer/CfsTlmFramer.hpp"

namespace FPrimeCfs {

class CfsTlmFramerTester final : public CfsTlmFramerGTestBase {
  public:
    // Maximum size of histories storing events, telemetry, and port outputs
    static const U32 MAX_HISTORY_SIZE = 100;

    // Instance ID supplied to the component instance under test
    static const FwEnumStoreType TEST_INSTANCE_ID = 0;

    //! Construct object CfsTlmFramerTester
    CfsTlmFramerTester();

    //! Destroy object CfsTlmFramerTester
    ~CfsTlmFramerTester();

    // ----------------------------------------------------------------------
    // Tests
    // ----------------------------------------------------------------------

    //! A telemetry packet is framed: secondary header flag, length, time, and payload
    void testFrameTelemetryPacket();

    //! A command packet passes through unmodified
    void testPassthroughCommandPacket();

    //! A telemetry packet already carrying a secondary header passes through unmodified
    void testPassthroughSecHdrTelemetry();

    //! A multi-packet buffer frames only the telemetry packets lacking a secondary header
    void testMultiplePackets();

    //! A malformed buffer is forwarded verbatim
    void testMalformedBuffer();

    //! An allocation failure drops the buffer, returns it, and keeps com status flowing
    void testAllocationFailure();

    //! A buffer returned on dataReturnIn is deallocated
    void testDataReturnDeallocate();

    //! Com status passes through the component
    void testComStatusPassthrough();

  private:
    // ----------------------------------------------------------------------
    // Helpers
    // ----------------------------------------------------------------------

    //! Write a space packet at the supplied location, returning its total size
    FwSizeType makePacket(U8* dest, bool command, bool secHdr, FwSizeType payloadSize, U8 fill);

    //! Invoke dataIn over the supplied bytes
    void sendData(U8* bytes, FwSizeType size);

    void connectPorts();    //!< Connects all ports for the component under test (auto-generated)
    void initComponents();  //!< Initializes the component under test (auto-generated)

    // ----------------------------------------------------------------------
    // Test Harness: output port overrides
    // ----------------------------------------------------------------------

    Fw::Buffer from_bufferAllocate_handler(FwIndexType portNum, FwSizeType size) override;

    //! The component under test
    CfsTlmFramer component;

    //! Allocation storage handed out by from_bufferAllocate_handler
    U8 m_allocStorage[4096] = {};

    //! Size limit returned by from_bufferAllocate_handler (simulates allocation failure)
    FwSizeType m_allocLimit = sizeof(CfsTlmFramerTester::m_allocStorage);
};

}  // namespace FPrimeCfs

#endif
