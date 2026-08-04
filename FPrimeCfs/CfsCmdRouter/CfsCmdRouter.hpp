// ======================================================================
// \title  CfsCmdRouter.hpp
// \brief  hpp file for CfsCmdRouter component implementation class
// ======================================================================

#ifndef FPrimeCfs_CfsCmdRouter_HPP
#define FPrimeCfs_CfsCmdRouter_HPP

#include "FPrimeCfs/CfsCmdRouter/CfsCmdRouterComponentAc.hpp"
#include "FPrimeCfs/CfsCmdRouter/CfsCmdRouter_CfsCmdRouteTableArrayAc.hpp"
#include "CfsCmdRouterConfig/FppConstantsAc.hpp"
#include "Fw/DataStructures/ArrayMap.hpp"
#include "Os/Mutex.hpp"

namespace FPrimeCfs {

//! Size in bytes of the cFS command secondary header ({U8 FunctionCode, U8 Checksum})
constexpr FwSizeType CFS_CMD_ROUTER_SEC_HDR_SIZE = 2;

//! Size in bytes of the CCSDS space packet primary header
constexpr FwSizeType CFS_CMD_ROUTER_PRI_HDR_SIZE = 6;

class CfsCmdRouter final : public CfsCmdRouterComponentBase {
  public:
    // ----------------------------------------------------------------------
    // Component construction and destruction
    // ----------------------------------------------------------------------

    //! Construct CfsCmdRouter object
    CfsCmdRouter(const char* const compName  //!< The component name
    );

    //! Destroy CfsCmdRouter object
    ~CfsCmdRouter();

  private:
    // ----------------------------------------------------------------------
    // Handler implementations for typed input ports
    // ----------------------------------------------------------------------

    //! Handler implementation for dataIn
    void dataIn_handler(FwIndexType portNum, Fw::Buffer& data, const ComCfg::FrameContext& context) override;

    //! Handler implementation for cmdResponseIn (no-op)
    void cmdResponseIn_handler(FwIndexType portNum,
                               FwOpcodeType opCode,
                               U32 cmdSeq,
                               const Fw::CmdResponse& response) override;

    //! Handler implementation for bufferReturnIn
    void bufferReturnIn_handler(FwIndexType portNum, Fw::Buffer& fwBuffer) override;

    // ----------------------------------------------------------------------
    // Helpers
    // ----------------------------------------------------------------------

    //! Compute the cFS command checksum residual over the message: the primary header
    //! is reconstructed from the context, then XORed with the data bytes and 0xFF.
    //! A valid checksum yields a residual of zero (per CFE_MSG conventions).
    static U8 computeChecksumResidual(const Fw::Buffer& data, const ComCfg::FrameContext& context);

    //! Look up the routing table entry for a function code; nullptr if not present
    const CfsCmdRouteEntry* findRoute(U8 functionCode) const;

    //! Return the incoming buffer to the sender with the context it was received with
    void returnData(Fw::Buffer& data, const ComCfg::FrameContext& context);

    //! Record of an outstanding pass-through transfer: the buffer originally received
    //! on dataIn and the context it was received with, keyed by the outgoing payload pointer
    struct PendingReturn {
        Fw::Buffer original;            //!< Buffer as received on dataIn
        ComCfg::FrameContext context;   //!< Context the buffer was received with
    };

    //! Track an outgoing pass-through buffer so the original buffer and context can be
    //! restored on return. On failure the buffer must not be routed.
    Fw::Success trackPending(const Fw::Buffer& outgoing,
                             const Fw::Buffer& original,
                             const ComCfg::FrameContext& context);

    //! Route a message as a bare com buffer without the secondary header (copy)
    void routeCom(const CfsCmdRouteEntry& route, Fw::Buffer& data, const ComCfg::FrameContext& context);

    //! Route a message as a function code and payload buffer (ownership transfer)
    void routeBuffer(const CfsCmdRouteEntry& route, Fw::Buffer& data, const ComCfg::FrameContext& context);

    //! Route a message to the unknown output (ownership transfer)
    void routeUnknown(Fw::Buffer& data, const ComCfg::FrameContext& context);

    // ----------------------------------------------------------------------
    // Member variables
    // ----------------------------------------------------------------------

    //! The function code routing table, statically configured via CfsCmdRouterCfg.fpp
    const CfsCmdRouter_CfsCmdRouteTable m_routes;

    //! Map of outstanding pass-through payload pointers to the original buffer and
    //! context, used to return the original buffer and context on dataReturnOut
    Fw::ArrayMap<const U8*, PendingReturn, CFS_CMD_ROUTER_MAX_PENDING_BUFFERS> m_pending;

    //! Protects m_pending; held only around map operations, never across port invocations
    Os::Mutex m_pendingLock;
};

}  // namespace FPrimeCfs

#endif
