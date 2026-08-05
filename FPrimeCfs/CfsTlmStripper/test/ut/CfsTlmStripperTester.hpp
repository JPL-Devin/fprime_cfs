// ======================================================================
// \title  CfsTlmStripperTester.hpp
// \brief  hpp file for CfsTlmStripper component test harness implementation class
// ======================================================================

#ifndef FPrimeCfs_CfsTlmStripperTester_HPP
#define FPrimeCfs_CfsTlmStripperTester_HPP

#include "CfsTlmStripperGTestBase.hpp"
#include "FPrimeCfs/CfsTlmStripper/CfsTlmStripper.hpp"

namespace FPrimeCfs {

class CfsTlmStripperTester final : public CfsTlmStripperGTestBase {
  public:
    // Maximum size of histories storing events, telemetry, and port outputs
    static const U32 MAX_HISTORY_SIZE = 10;

    // Instance ID supplied to the component instance under test
    static const FwEnumStoreType TEST_INSTANCE_ID = 0;

    //! Construct object CfsTlmStripperTester
    CfsTlmStripperTester();

    //! Destroy object CfsTlmStripperTester
    ~CfsTlmStripperTester();

    // ----------------------------------------------------------------------
    // Tests
    // ----------------------------------------------------------------------

    //! A telemetry packet with a secondary header is stripped in place
    void testTelemetryStrip();

    //! A command packet is forwarded unchanged
    void testCommandPassthrough();

    //! A telemetry packet without the secondary header flag is forwarded unchanged
    void testNoSecHdrPassthrough();

    //! A buffer too small for a primary header is dropped with a warning
    void testMalformedPacket();

    //! dataReturnIn is passed through to dataReturnOut
    void testDataReturnPassthrough();

    //! comStatusIn is passed through to comStatusOut
    void testComStatusPassthrough();

  private:
    // ----------------------------------------------------------------------
    // Helpers
    // ----------------------------------------------------------------------

    //! Build a space packet in m_packetStorage: primary header (streamId flags,
    //! APID, sequence, length), optional 6-byte secondary header, and payload
    Fw::Buffer buildPacket(bool command, bool secHdr, FwSizeType payloadSize);

    void connectPorts();    //!< Connects all ports for the component under test (auto-generated)
    void initComponents();  //!< Initializes the component under test (auto-generated)

    // ----------------------------------------------------------------------
    // Member variables
    // ----------------------------------------------------------------------

    //! The component under test
    CfsTlmStripper component;

    //! Storage for test packets
    U8 m_packetStorage[128] = {};
};

}  // namespace FPrimeCfs

#endif
