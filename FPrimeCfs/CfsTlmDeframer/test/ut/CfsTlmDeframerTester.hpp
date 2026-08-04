// ======================================================================
// \title  CfsTlmDeframerTester.hpp
// \brief  hpp file for CfsTlmDeframer component test harness implementation class
// ======================================================================

#ifndef FPrimeCfs_CfsTlmDeframerTester_HPP
#define FPrimeCfs_CfsTlmDeframerTester_HPP

#include "CfsTlmDeframerGTestBase.hpp"
#include "FPrimeCfs/CfsTlmDeframer/CfsTlmDeframer.hpp"

namespace FPrimeCfs {

class CfsTlmDeframerTester final : public CfsTlmDeframerGTestBase {
  public:
    // Maximum size of histories storing events, telemetry, and port outputs
    static const U32 MAX_HISTORY_SIZE = 100;

    // Instance ID supplied to the component instance under test
    static const FwEnumStoreType TEST_INSTANCE_ID = 0;

    //! Construct object CfsTlmDeframerTester
    CfsTlmDeframerTester();

    //! Destroy object CfsTlmDeframerTester
    ~CfsTlmDeframerTester();

    // ----------------------------------------------------------------------
    // Tests
    // ----------------------------------------------------------------------

    //! A telemetry packet with a secondary header is stripped: flag, length, and payload
    void testStripTelemetrySecHdr();

    //! A telemetry packet without a secondary header passes through unmodified
    void testPassthroughBareTelemetry();

    //! A command packet passes through unmodified
    void testPassthroughCommandPacket();

    //! A packet too short to strip passes through unmodified
    void testPassthroughShortPacket();

    //! A buffer returned on dataReturnIn is returned upstream
    void testDataReturnPassthrough();

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

    //! The component under test
    CfsTlmDeframer component;
};

}  // namespace FPrimeCfs

#endif
