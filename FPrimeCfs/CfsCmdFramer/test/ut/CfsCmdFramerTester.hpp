// ======================================================================
// \title  CfsCmdFramerTester.hpp
// \brief  hpp file for CfsCmdFramer component test harness implementation class
// ======================================================================

#ifndef FPrimeCfs_CfsCmdFramerTester_HPP
#define FPrimeCfs_CfsCmdFramerTester_HPP

#include "CfsCmdFramerGTestBase.hpp"
#include "FPrimeCfs/CfsCmdFramer/CfsCmdFramer.hpp"

namespace FPrimeCfs {

class CfsCmdFramerTester final : public CfsCmdFramerGTestBase {
  public:
    // Maximum size of histories storing events, telemetry, and port outputs
    static const U32 MAX_HISTORY_SIZE = 10;

    // Instance ID supplied to the component instance under test
    static const FwEnumStoreType TEST_INSTANCE_ID = 0;

    //! Construct object CfsCmdFramerTester
    CfsCmdFramerTester();

    //! Destroy object CfsCmdFramerTester
    ~CfsCmdFramerTester();

    // ----------------------------------------------------------------------
    // Tests
    // ----------------------------------------------------------------------

    //! Nominal wrapping: secondary header prepended, context flag set, data returned
    void testNominalFraming();

    //! The checksum makes the predicted complete packet XOR to zero
    void testChecksum();

    //! An allocation that is too small drops the packet and returns the data
    void testAllocationFailure();

    //! comStatusIn is passed through to comStatusOut
    void testComStatusPassthrough();

    //! dataReturnIn deallocates the wrapped buffer
    void testDataReturnPassthrough();

  private:
    // ----------------------------------------------------------------------
    // Output port handler overrides
    // ----------------------------------------------------------------------

    //! Handler for bufferAllocate
    Fw::Buffer from_bufferAllocate_handler(FwIndexType portNum, FwSizeType size) override;

    // ----------------------------------------------------------------------
    // Helpers
    // ----------------------------------------------------------------------

    //! Connect ports
    void connectPorts();

    //! Initialize components
    void initComponents();

    // ----------------------------------------------------------------------
    // Member variables
    // ----------------------------------------------------------------------

    //! The component under test
    CfsCmdFramer component;

    //! Backing storage returned by the bufferAllocate handler
    U8 m_allocStorage[1024] = {};

    //! Size to return from the bufferAllocate handler (0 means requested size)
    FwSizeType m_allocSize = 0;

    //! Whether to force an undersized allocation
    bool m_useUndersizedAlloc = false;
};

}  // namespace FPrimeCfs

#endif
