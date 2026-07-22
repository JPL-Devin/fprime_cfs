// ======================================================================
// \title  SchAppDriverTester.hpp
// \author mstarch
// \brief  hpp file for SchAppDriver component test harness implementation class
// ======================================================================

#ifndef FPrimeCfs_SchAppDriverTester_HPP
#define FPrimeCfs_SchAppDriverTester_HPP

#include "FPrimeCfs/SchAppDriver/SchAppDriver.hpp"
#include "FPrimeCfs/SchAppDriver/SchAppDriverGTestBase.hpp"

namespace FPrimeCfs {

class SchAppDriverTester final : public SchAppDriverGTestBase {
  public:
    // ----------------------------------------------------------------------
    // Constants
    // ----------------------------------------------------------------------

    // Maximum size of histories storing events, telemetry, and port outputs
    static const FwSizeType MAX_HISTORY_SIZE = 10;

    // Instance ID supplied to the component instance under test
    static const FwEnumStoreType TEST_INSTANCE_ID = 0;

    // Maximum size of the test message buffer
    static const FwSizeType MAX_MESSAGE_SIZE = 64;

  public:
    // ----------------------------------------------------------------------
    // Construction and destruction
    // ----------------------------------------------------------------------

    //! Construct object SchAppDriverTester
    SchAppDriverTester();

    //! Destroy object SchAppDriverTester
    ~SchAppDriverTester();

  public:
    // ----------------------------------------------------------------------
    // Tests
    // ----------------------------------------------------------------------

    //! A single scheduler message produces a single tick and buffer return
    void testSingleTick();

    //! Each of a series of scheduler messages produces one tick and buffer return
    void testMultipleTicks();

  private:
    // ----------------------------------------------------------------------
    // Helper functions
    // ----------------------------------------------------------------------

    //! Send a scheduler message with random contents on dataIn
    void sendSchMessage(Fw::Buffer& buffer, ComCfg::FrameContext& context);

    //! Assert exactly one tick and one buffer return matching the sent message
    void assertSingleTickAndReturn(const Fw::Buffer& buffer, const ComCfg::FrameContext& context);

    //! Connect ports
    void connectPorts();

    //! Initialize components
    void initComponents();

  private:
    // ----------------------------------------------------------------------
    // Member variables
    // ----------------------------------------------------------------------

    //! The component under test
    SchAppDriver component;

    //! Backing storage for test message buffers
    U8 m_messageData[MAX_MESSAGE_SIZE];
};

}  // namespace FPrimeCfs

#endif
