// ======================================================================
// \title  CfsSystemTimeTester.hpp
// \brief  hpp file for CfsSystemTime component test harness implementation class
// ======================================================================

#ifndef FPrimeCfs_CfsSystemTimeTester_HPP
#define FPrimeCfs_CfsSystemTimeTester_HPP

#include "FPrimeCfs/CfsSystemTime/CfsSystemTime.hpp"
#include "FPrimeCfs/CfsSystemTime/CfsSystemTimeGTestBase.hpp"

namespace FPrimeCfs {

class CfsSystemTimeTester : public CfsSystemTimeGTestBase {
  public:
    // ----------------------------------------------------------------------
    // Constants
    // ----------------------------------------------------------------------

    // Maximum size of histories storing events, telemetry, and port outputs
    static const FwSizeType MAX_HISTORY_SIZE = 10;

    // Instance ID supplied to the component instance under test
    static const FwEnumStoreType TEST_INSTANCE_ID = 0;

  public:
    // ----------------------------------------------------------------------
    // Construction and destruction
    // ----------------------------------------------------------------------

    //! Construct object CfsSystemTimeTester
    CfsSystemTimeTester();

    //! Destroy object CfsSystemTimeTester
    ~CfsSystemTimeTester();

  public:
    // ----------------------------------------------------------------------
    // Tests
    // ----------------------------------------------------------------------

    //! Test that time retrieval returns the cFS time as an Fw::Time
    void test_get_time();

    //! Test that Fw::Time converts back to a cFS system time
    void test_convert_time();

  private:
    // ----------------------------------------------------------------------
    // Helper functions
    // ----------------------------------------------------------------------

    //! Connect ports
    void connectPorts();

    //! Initialize components
    void initComponents();

  private:
    // ----------------------------------------------------------------------
    // Member variables
    // ----------------------------------------------------------------------

    //! The component under test
    CfsSystemTime component;
};

}  // namespace FPrimeCfs

#endif
