// ======================================================================
// \title  EvsMirrorTester.hpp
// \brief  hpp file for EvsMirror component test harness implementation class
// ======================================================================

#ifndef FPrimeCfs_EvsMirrorTester_HPP
#define FPrimeCfs_EvsMirrorTester_HPP

#include "FPrimeCfs/EvsMirror/EvsMirror.hpp"
#include "FPrimeCfs/EvsMirror/EvsMirrorGTestBase.hpp"

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

    //! Events on logIn are forwarded unchanged on logOut
    void testLogPassThrough();

    //! Text events on textLogIn are forwarded unchanged on textLogOut and
    //! mirrored to EVS with the formatted text and truncated event ID
    void testTextLogPassThroughAndMirror();

    //! Each F Prime severity maps to the expected EVS event type
    void testSeverityMapping();

    //! An EVS send failure does not prevent forwarding
    void testEvsFailureStillForwards();

  private:
    // ----------------------------------------------------------------------
    // Helper functions
    // ----------------------------------------------------------------------

    //! Send a text event with the given ID and severity and random text
    void sendTextEvent(FwEventIdType id, const Fw::LogSeverity& severity, Fw::TextLogString& text);

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
