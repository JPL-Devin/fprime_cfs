// ======================================================================
// \title  EvsMirrorTester.hpp
// \brief  hpp file for EvsMirror component test harness implementation class
// ======================================================================

#ifndef FPrimeCfs_EvsMirrorTester_HPP
#define FPrimeCfs_EvsMirrorTester_HPP

#include "FPrimeCfs/EvsMirror/EvsMirror.hpp"
#include "FPrimeCfs/EvsMirror/EvsMirrorGTestBase.hpp"
#include "Fw/Logger/Logger.hpp"

namespace FPrimeCfs {

class EvsMirrorTester final : public EvsMirrorGTestBase {
  public:
    // ----------------------------------------------------------------------
    // Constants
    // ----------------------------------------------------------------------

    // Maximum size of histories storing events, telemetry, and port outputs
    static const FwSizeType MAX_HISTORY_SIZE = 100;

    // Instance ID supplied to the component instance under test
    static const FwEnumStoreType TEST_INSTANCE_ID = 0;

  public:
    // ----------------------------------------------------------------------
    // Construction and destruction
    // ----------------------------------------------------------------------

    //! Construct object EvsMirrorTester
    EvsMirrorTester();

    //! Destroy object EvsMirrorTester
    ~EvsMirrorTester();

  public:
    // ----------------------------------------------------------------------
    // Tests
    // ----------------------------------------------------------------------

    //! Text events are mirrored to EVS with the formatted text and truncated
    //! event ID
    void testMirror();

    //! Each F Prime severity maps to the expected EVS event type
    void testSeverityMapping();

    //! An EVS send failure is tolerated without asserting
    void testEvsFailure();

  private:
    // ----------------------------------------------------------------------
    // Helper functions
    // ----------------------------------------------------------------------

    //! Logger counting Fw::Logger messages for assertions on error logging
    class CountingLogger final : public Fw::Logger {
      public:
        U32 messageCount = 0;

      protected:
        void writeMessage(const Fw::ConstStringBase& message) override { this->messageCount++; }
    };

    //! Send a text event with the given ID and severity; fills generatedText with
    //! the random text that was sent, for later assertions
    void sendTextEvent(FwEventIdType id, const Fw::LogSeverity& severity, Fw::TextLogString& generatedText);

    //! Connect ports
    void connectPorts();

    //! Initialize components
    void initComponents();

  private:
    // ----------------------------------------------------------------------
    // Member variables
    // ----------------------------------------------------------------------

    //! The component under test
    EvsMirror component;
};

}  // namespace FPrimeCfs

#endif
