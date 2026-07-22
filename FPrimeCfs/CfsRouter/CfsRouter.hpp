// ======================================================================
// \title  CfsRouter.hpp
// \brief  hpp file for CfsRouter component implementation class
// ======================================================================

#ifndef FPrimeCfs_CfsRouter_HPP
#define FPrimeCfs_CfsRouter_HPP

#include "FPrimeCfs/CfsRouter/CfsRouteTypeEnumAc.hpp"
#include "FPrimeCfs/CfsRouter/CfsRouterComponentAc.hpp"

namespace FPrimeCfs {

//! Size in bytes of the cFS command secondary header ({U8 FunctionCode, U8 Checksum})
constexpr FwSizeType CFS_ROUTER_CMD_SEC_HDR_SIZE = 2;

//! Size in bytes of the cFS telemetry secondary header (4-byte seconds, 2-byte subseconds)
constexpr FwSizeType CFS_ROUTER_TLM_SEC_HDR_SIZE = 6;

//! An entry in the APID routing table
struct CfsRouteEntry {
    ComCfg::Apid::T apid;   //!< The APID to route
    CfsRouteType::T type;   //!< The category of output port to route to
    FwIndexType index;      //!< The index within that category's output port array
};

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

    //! Supply the APID routing table. The table must remain valid for the
    //! lifetime of the component (e.g. a statically allocated table).
    void configure(const CfsRouteEntry* table,  //!< The routing table
                   FwSizeType entries           //!< The number of entries in the table
    );

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

    //! Return the incoming buffer to the sender with an empty context
    void returnData(Fw::Buffer& data);

    //! Route an F Prime command packet (copy)
    void routeFprimeCommand(const CfsRouteEntry& route, Fw::Buffer& data, const ComCfg::FrameContext& context);

    //! Route a cFS command packet (ownership transfer)
    void routeCfsCommand(const CfsRouteEntry& route, Fw::Buffer& data, const ComCfg::FrameContext& context);

    //! Route a cFS telemetry packet (ownership transfer)
    void routeCfsTelemetry(const CfsRouteEntry& route, Fw::Buffer& data, const ComCfg::FrameContext& context);

    //! Route a packet to the unknown output (ownership transfer)
    void routeUnknown(Fw::Buffer& data, const ComCfg::FrameContext& context);

    // ----------------------------------------------------------------------
    // Member variables
    // ----------------------------------------------------------------------

    const CfsRouteEntry* m_table;  //!< The APID routing table
    FwSizeType m_entries;          //!< The number of entries in the table
};

}  // namespace FPrimeCfs

#endif
