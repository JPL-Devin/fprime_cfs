// ======================================================================
// \title  CfsRouter.hpp
// \brief  hpp file for CfsRouter component implementation class
// ======================================================================

#ifndef FPrimeCfs_CfsRouter_HPP
#define FPrimeCfs_CfsRouter_HPP

#include "FPrimeCfs/CfsRouter/CfsRouterComponentAc.hpp"
#include "FPrimeCfs/CfsRouter/CfsRouter_CfsRouteTableArrayAc.hpp"
#include "CfsRouterConfig/FppConstantsAc.hpp"
#include "Fw/DataStructures/ArrayMap.hpp"

namespace FPrimeCfs {

//! Size in bytes of the cFS command secondary header ({U8 FunctionCode, U8 Checksum})
constexpr FwSizeType CFS_ROUTER_CMD_SEC_HDR_SIZE = 2;

//! Size in bytes of the cFS telemetry secondary header (4-byte seconds, 2-byte subseconds)
constexpr FwSizeType CFS_ROUTER_TLM_SEC_HDR_SIZE = 6;

class CfsRouter final : public CfsRouterComponentBase {
  public:
    // ----------------------------------------------------------------------
    // Component construction and destruction
    // ----------------------------------------------------------------------

    //! Construct CfsRouter object
    CfsRouter(const char* const compName  //!< The component name
    );

    //! Destroy CfsRouter object
    ~CfsRouter();

  private:
    // ----------------------------------------------------------------------
    // Handler implementations for typed input ports
    // ----------------------------------------------------------------------

    //! Handler implementation for dataIn
    void dataIn_handler(FwIndexType portNum,
                        Fw::Buffer& data,
                        const ComCfg::FrameContext& context) override;

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

    //! Look up the routing table entry for an APID; nullptr if not present
    const CfsRouteEntry* findRoute(ComCfg::Apid::T apid) const;

    //! Return the incoming buffer to the sender with the context it was received with
    void returnData(Fw::Buffer& data, const ComCfg::FrameContext& context);

    //! Track an outgoing pass-through buffer so its context can be restored on return.
    //! Returns true on success; on failure the buffer must not be routed.
    bool trackPending(const Fw::Buffer& buffer, const ComCfg::FrameContext& context);

    //! Route an F Prime command packet (copy)
    void routeFprimeCommand(const CfsRouteEntry& route,
                            Fw::Buffer& data,
                            const ComCfg::FrameContext& context);

    //! Route a cFS command packet (ownership transfer)
    void routeCfsCommand(const CfsRouteEntry& route,
                         Fw::Buffer& data,
                         const ComCfg::FrameContext& context);

    //! Route a cFS telemetry packet (ownership transfer)
    void routeCfsTelemetry(const CfsRouteEntry& route,
                           Fw::Buffer& data,
                           const ComCfg::FrameContext& context);

    //! Route a packet to the unknown output (ownership transfer)
    void routeUnknown(Fw::Buffer& data, const ComCfg::FrameContext& context);

    // ----------------------------------------------------------------------
    // Member variables
    // ----------------------------------------------------------------------

    //! The APID routing table, statically configured via CfsRouterCfg.fpp
    CfsRouter_CfsRouteTable m_routes;

    //! Map of outstanding pass-through buffers to the context each was received with,
    //! used to return the original context on dataReturnOut
    Fw::ArrayMap<const U8*, ComCfg::FrameContext, CFS_ROUTER_MAX_PENDING_BUFFERS> m_pending;
};

}  // namespace FPrimeCfs

#endif
