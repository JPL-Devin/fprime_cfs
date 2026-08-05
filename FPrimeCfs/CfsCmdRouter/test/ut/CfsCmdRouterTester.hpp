// ======================================================================
// \title  CfsCmdRouterTester.hpp
// \brief  hpp file for CfsCmdRouter component test harness implementation class
// ======================================================================

#ifndef FPrimeCfs_CfsCmdRouterTester_HPP
#define FPrimeCfs_CfsCmdRouterTester_HPP

#include "CfsCmdRouterGTestBase.hpp"
#include "FPrimeCfs/CfsCmdRouter/CfsCmdRouter.hpp"

namespace FPrimeCfs {

class CfsCmdRouterTester final : public CfsCmdRouterGTestBase {
  public:
    // Maximum size of histories storing events, telemetry, and port outputs
    static const U32 MAX_HISTORY_SIZE = 100;

    // Instance ID supplied to the component instance under test
    static const FwEnumStoreType TEST_INSTANCE_ID = 0;

    //! Construct object CfsCmdRouterTester. When connectOutputs is false, no
    //! route output ports are connected (dataReturnOut is always connected).
    explicit CfsCmdRouterTester(bool connectOutputs = true);

    //! Destroy object CfsCmdRouterTester
    ~CfsCmdRouterTester();

    // ----------------------------------------------------------------------
    // Tests
    // ----------------------------------------------------------------------

    //! Com-routed function codes are copied without the secondary header and the buffer returned
    void testRouteCom();

    //! Oversized com data emits a serialization error and returns the buffer
    void testRouteComTooLarge();

    //! Buffer-routed function codes pass the function code and payload; ownership returns via bufferReturnIn
    void testRouteBuffer();

    //! Unconfigured function codes route to the unknown output with their context
    void testRouteUnknown();

    //! Messages failing the checksum are dropped with a warning and the buffer returned
    void testBadChecksum();

    //! Messages without a secondary header go to unknown with a warning event
    void testMissingSecondaryHeader();

    //! Messages whose data is too small for the secondary header go to unknown with a warning
    void testShortSecondaryHeader();

    //! Disconnected route outputs return the buffer rather than assert or leak
    void testDisconnectedOutputs();

    //! Untracked buffers returned on bufferReturnIn are forwarded as-is with a default context
    void testBufferReturnUntracked();

    //! cmdResponseIn is accepted as a no-op
    void testCommandResponseNoop();

    //! Randomized routing across all categories returns every buffer
    void testRandomized();

  private:
    // ----------------------------------------------------------------------
    // Helpers
    // ----------------------------------------------------------------------

    //! Set the checksum byte of a message so it validates against the reconstructed
    //! primary header for the given apid/sequence count
    static void setValidChecksum(ComCfg::Apid::T apid, U16 sequenceCount, U8* bytes, FwSizeType size);

    //! Invoke dataIn with the given apid/secondary-header flag over the supplied bytes
    void sendData(ComCfg::Apid::T apid, bool hasSecHdr, U8* bytes, FwSizeType size, U16 sequenceCount = 0);

    void connectPorts();    //!< Connects all ports for the component under test (auto-generated)
    void initComponents();  //!< Initializes the component under test (auto-generated)

    //! The component under test
    CfsCmdRouter component;
};

}  // namespace FPrimeCfs

#endif
