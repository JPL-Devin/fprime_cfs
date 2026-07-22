// ======================================================================
// \title  CfsRouter.cpp
// \brief  cpp file for CfsRouter component implementation class
// ======================================================================

#include "FPrimeCfs/CfsRouter/CfsRouter.hpp"
#include "Fw/FPrimeBasicTypes.hpp"

namespace FPrimeCfs {

// ----------------------------------------------------------------------
// Component construction and destruction
// ----------------------------------------------------------------------

CfsRouter ::CfsRouter(const char* const compName)
    : CfsRouterComponentBase(compName), m_table(nullptr), m_entries(0) {}

CfsRouter ::~CfsRouter() {}

void CfsRouter ::configure(const CfsRouteEntry* table, FwSizeType entries) {
    FW_ASSERT((table != nullptr) || (entries == 0));
    this->m_table = table;
    this->m_entries = entries;
}

// ----------------------------------------------------------------------
// Handler implementations for typed input ports
// ----------------------------------------------------------------------

void CfsRouter ::dataIn_handler(FwIndexType portNum, Fw::Buffer& data, const ComCfg::FrameContext& context) {
    const CfsRouteEntry* route = this->findRoute(context.get_apid());
    if (route == nullptr) {
        this->routeUnknown(data, context);
        return;
    }
    switch (route->type) {
        case CfsRouteType::FPRIME_COMMAND:
            this->routeFprimeCommand(*route, data, context);
            break;
        case CfsRouteType::CFS_COMMAND:
            this->routeCfsCommand(*route, data, context);
            break;
        case CfsRouteType::CFS_TELEMETRY:
            this->routeCfsTelemetry(*route, data, context);
            break;
        default:
            FW_ASSERT(0, static_cast<FwAssertArgType>(route->type));
            break;
    }
}

void CfsRouter ::cmdResponseIn_handler(FwIndexType portNum,
                                       FwOpcodeType opCode,
                                       U32 cmdSeq,
                                       const Fw::CmdResponse& response) {
    // Nothing to do
}

void CfsRouter ::bufferReturnIn_handler(FwIndexType portNum, Fw::Buffer& fwBuffer) {
    // Ownership of a buffer sent on cfsCommandOut, cfsTelemetryOut, or unknownDataOut
    // has been returned; complete the transfer back to the deframer
    this->returnData(fwBuffer);
}

// ----------------------------------------------------------------------
// Helpers
// ----------------------------------------------------------------------

const CfsRouteEntry* CfsRouter ::findRoute(ComCfg::Apid::T apid) const {
    for (FwSizeType i = 0; i < this->m_entries; i++) {
        if (this->m_table[i].apid == apid) {
            return &this->m_table[i];
        }
    }
    return nullptr;
}

void CfsRouter ::returnData(Fw::Buffer& data) {
    ComCfg::FrameContext emptyContext;
    this->dataReturnOut_out(0, data, emptyContext);
}

void CfsRouter ::routeFprimeCommand(const CfsRouteEntry& route,
                                    Fw::Buffer& data,
                                    const ComCfg::FrameContext& context) {
    // A cFS-framed F Prime command carries the cFS command secondary header ahead of
    // the F Prime command data; exclude it from the copy when present
    const FwSizeType offset = context.get_hasSecHdr() ? CFS_ROUTER_CMD_SEC_HDR_SIZE : 0;
    if ((data.getSize() >= offset) && this->isConnected_commandOut_OutputPort(route.index)) {
        Fw::ComBuffer com;
        const Fw::SerializeStatus status = com.setBuff(data.getData() + offset, data.getSize() - offset);
        if (status == Fw::FW_SERIALIZE_OK) {
            this->commandOut_out(route.index, com, 0);
        } else {
            this->log_WARNING_HI_SerializationError(static_cast<U32>(status));
        }
    } else if (data.getSize() < offset) {
        this->log_WARNING_HI_MissingSecondaryHeader(static_cast<U16>(context.get_apid()),
                                                    static_cast<U32>(data.getSize()));
    }
    // The F Prime command route copies: ownership returns to the sender immediately
    this->returnData(data);
}

void CfsRouter ::routeCfsCommand(const CfsRouteEntry& route, Fw::Buffer& data, const ComCfg::FrameContext& context) {
    if ((!context.get_hasSecHdr()) || (data.getSize() < CFS_ROUTER_CMD_SEC_HDR_SIZE)) {
        this->log_WARNING_HI_MissingSecondaryHeader(static_cast<U16>(context.get_apid()),
                                                    static_cast<U32>(data.getSize()));
        this->routeUnknown(data, context);
        return;
    }
    if (!this->isConnected_cfsCommandOut_OutputPort(route.index)) {
        this->returnData(data);
        return;
    }
    const U8 functionCode = data.getData()[0];
    // Payload is the data after the secondary header; ownership transfers to the receiver
    // and returns via bufferReturnIn
    Fw::Buffer payload(data.getData() + CFS_ROUTER_CMD_SEC_HDR_SIZE, data.getSize() - CFS_ROUTER_CMD_SEC_HDR_SIZE);
    this->cfsCommandOut_out(route.index, functionCode, payload);
}

void CfsRouter ::routeCfsTelemetry(const CfsRouteEntry& route, Fw::Buffer& data, const ComCfg::FrameContext& context) {
    if ((!context.get_hasSecHdr()) || (data.getSize() < CFS_ROUTER_TLM_SEC_HDR_SIZE)) {
        this->log_WARNING_HI_MissingSecondaryHeader(static_cast<U16>(context.get_apid()),
                                                    static_cast<U32>(data.getSize()));
        this->routeUnknown(data, context);
        return;
    }
    if (!this->isConnected_cfsTelemetryOut_OutputPort(route.index)) {
        this->returnData(data);
        return;
    }
    // The cFS telemetry secondary header is big-endian: 4-byte seconds, 2-byte subseconds.
    // The 16-bit subseconds field holds the most significant 16 bits of the 2^-32 value.
    const U8* const bytes = data.getData();
    const U32 seconds = (static_cast<U32>(bytes[0]) << 24) | (static_cast<U32>(bytes[1]) << 16) |
                        (static_cast<U32>(bytes[2]) << 8) | static_cast<U32>(bytes[3]);
    const U32 subseconds = ((static_cast<U32>(bytes[4]) << 8) | static_cast<U32>(bytes[5])) << 16;
    const CfsTime time(seconds, subseconds);
    // Payload is the data after the secondary header; ownership transfers to the receiver
    // and returns via bufferReturnIn
    Fw::Buffer payload(data.getData() + CFS_ROUTER_TLM_SEC_HDR_SIZE, data.getSize() - CFS_ROUTER_TLM_SEC_HDR_SIZE);
    this->cfsTelemetryOut_out(route.index, time, payload);
}

void CfsRouter ::routeUnknown(Fw::Buffer& data, const ComCfg::FrameContext& context) {
    if (this->isConnected_unknownDataOut_OutputPort(0)) {
        // Ownership transfers to the receiver and returns via bufferReturnIn
        this->unknownDataOut_out(0, data, context);
    } else {
        this->returnData(data);
    }
}

}  // namespace FPrimeCfs
