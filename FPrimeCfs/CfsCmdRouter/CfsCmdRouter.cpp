// ======================================================================
// \title  CfsCmdRouter.cpp
// \brief  cpp file for CfsCmdRouter component implementation class
// ======================================================================

#include "FPrimeCfs/CfsCmdRouter/CfsCmdRouter.hpp"
#include "Fw/FPrimeBasicTypes.hpp"

namespace FPrimeCfs {

// ----------------------------------------------------------------------
// Component construction and destruction
// ----------------------------------------------------------------------

CfsCmdRouter ::CfsCmdRouter(const char* const compName) : CfsCmdRouterComponentBase(compName) {}

CfsCmdRouter ::~CfsCmdRouter() {}

// ----------------------------------------------------------------------
// Handler implementations for typed input ports
// ----------------------------------------------------------------------

void CfsCmdRouter ::dataIn_handler(FwIndexType portNum, Fw::Buffer& data, const ComCfg::FrameContext& context) {
    if ((!context.get_hasSecHdr()) || (data.getSize() < CFS_CMD_ROUTER_SEC_HDR_SIZE)) {
        this->log_WARNING_HI_MissingSecondaryHeader(static_cast<U16>(context.get_apid()),
                                                    static_cast<U32>(data.getSize()));
        this->routeUnknown(data, context);
        return;
    }
    const U8 functionCode = data.getData()[0];
    const U8 residual = CfsCmdRouter::computeChecksumResidual(data, context);
    if (residual != 0) {
        // Failed checksum: drop the message and return the buffer to the sender
        this->log_WARNING_HI_BadChecksum(static_cast<U16>(context.get_apid()), functionCode, residual);
        this->returnData(data, context);
        return;
    }
    const CfsCmdRouteEntry* route = this->findRoute(functionCode);
    if (route == nullptr) {
        this->routeUnknown(data, context);
        return;
    }
    switch (route->get_routeType()) {
        case CfsCmdRouteType::COM:
            this->routeCom(*route, data, context);
            break;
        case CfsCmdRouteType::BUFFER:
            this->routeBuffer(*route, data, context);
            break;
        default:
            FW_ASSERT(0, static_cast<FwAssertArgType>(route->get_routeType()));
            break;
    }
}

void CfsCmdRouter ::cmdResponseIn_handler(FwIndexType portNum,
                                          FwOpcodeType opCode,
                                          U32 cmdSeq,
                                          const Fw::CmdResponse& response) {
    // Nothing to do
}

void CfsCmdRouter ::bufferReturnIn_handler(FwIndexType portNum, Fw::Buffer& fwBuffer) {
    // Ownership of a buffer sent on bufferOut or unknownDataOut has been returned;
    // complete the transfer back to the sender with the context the data was
    // originally received with
    ComCfg::FrameContext context;
    (void)this->m_pending.remove(fwBuffer.getData(), context);
    this->returnData(fwBuffer, context);
}

// ----------------------------------------------------------------------
// Helpers
// ----------------------------------------------------------------------

U8 CfsCmdRouter ::computeChecksumResidual(const Fw::Buffer& data, const ComCfg::FrameContext& context) {
    // The cFS command checksum is computed over the complete space packet: XOR of every
    // packet byte with 0xFF equals zero for a valid message (per CFE_MSG conventions).
    // The primary header was consumed by the deframer, so reconstruct it from the
    // context: PVN 0, packet type command (1), secondary header present (1), the APID,
    // sequence flags unsegmented (0b11), the sequence count, and the length field
    // (number of packet data bytes minus one).
    const U16 apid = static_cast<U16>(context.get_apid());
    const U16 sequenceCount = context.get_sequenceCount();
    const FwSizeType lengthToken = data.getSize() - 1;
    U8 residual = 0xFF;
    residual ^= static_cast<U8>(0x18 | ((apid >> 8) & 0x07));
    residual ^= static_cast<U8>(apid & 0xFF);
    residual ^= static_cast<U8>(0xC0 | ((sequenceCount >> 8) & 0x3F));
    residual ^= static_cast<U8>(sequenceCount & 0xFF);
    residual ^= static_cast<U8>((lengthToken >> 8) & 0xFF);
    residual ^= static_cast<U8>(lengthToken & 0xFF);
    const U8* const bytes = data.getData();
    for (FwSizeType i = 0; i < data.getSize(); i++) {
        residual ^= bytes[i];
    }
    return residual;
}

const CfsCmdRouteEntry* CfsCmdRouter ::findRoute(U8 functionCode) const {
    for (FwSizeType i = 0; i < CfsCmdRouter_CfsCmdRouteTable::SIZE; i++) {
        if (this->m_routes[i].get_functionCode() == functionCode) {
            return &this->m_routes[i];
        }
    }
    return nullptr;
}

void CfsCmdRouter ::returnData(Fw::Buffer& data, const ComCfg::FrameContext& context) {
    this->dataReturnOut_out(0, data, context);
}

Fw::Success CfsCmdRouter ::trackPending(const Fw::Buffer& buffer, const ComCfg::FrameContext& context) {
    const Fw::Success status = this->m_pending.insert(buffer.getData(), context);
    if (status != Fw::Success::SUCCESS) {
        this->log_WARNING_HI_TooManyPendingBuffers(static_cast<U16>(context.get_apid()));
    }
    return status;
}

void CfsCmdRouter ::routeCom(const CfsCmdRouteEntry& route, Fw::Buffer& data, const ComCfg::FrameContext& context) {
    const FwIndexType index = route.get_portIndex();
    if (this->isConnected_comOut_OutputPort(index)) {
        Fw::ComBuffer com;
        const Fw::SerializeStatus status = com.setBuff(data.getData() + CFS_CMD_ROUTER_SEC_HDR_SIZE,
                                                       data.getSize() - CFS_CMD_ROUTER_SEC_HDR_SIZE);
        if (status == Fw::FW_SERIALIZE_OK) {
            this->comOut_out(index, com, 0);
        } else {
            this->log_WARNING_HI_SerializationError(static_cast<U32>(status));
        }
    }
    // The com route copies: ownership returns to the sender immediately
    this->returnData(data, context);
}

void CfsCmdRouter ::routeBuffer(const CfsCmdRouteEntry& route, Fw::Buffer& data, const ComCfg::FrameContext& context) {
    const FwIndexType index = route.get_portIndex();
    if (!this->isConnected_bufferOut_OutputPort(index)) {
        this->returnData(data, context);
        return;
    }
    const U8 functionCode = data.getData()[0];
    // Payload is the data after the secondary header; ownership transfers to the receiver
    // and returns via bufferReturnIn
    Fw::Buffer payload(data.getData() + CFS_CMD_ROUTER_SEC_HDR_SIZE, data.getSize() - CFS_CMD_ROUTER_SEC_HDR_SIZE);
    const Fw::Success trackStatus = this->trackPending(payload, context);
    if (trackStatus == Fw::Success::SUCCESS) {
        this->bufferOut_out(index, functionCode, payload);
    } else {
        this->returnData(data, context);
    }
}

void CfsCmdRouter ::routeUnknown(Fw::Buffer& data, const ComCfg::FrameContext& context) {
    if (!this->isConnected_unknownDataOut_OutputPort(0)) {
        this->returnData(data, context);
        return;
    }
    const Fw::Success trackStatus = this->trackPending(data, context);
    if (trackStatus == Fw::Success::SUCCESS) {
        // Ownership transfers to the receiver and returns via bufferReturnIn
        this->unknownDataOut_out(0, data, context);
    } else {
        this->returnData(data, context);
    }
}

}  // namespace FPrimeCfs
