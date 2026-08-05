// ======================================================================
// \title  CfsTlmStripper.cpp
// \brief  cpp file for CfsTlmStripper component implementation class
// ======================================================================

#include "FPrimeCfs/CfsTlmStripper/CfsTlmStripper.hpp"
#include "Fw/FPrimeBasicTypes.hpp"
#include <cstring>

namespace FPrimeCfs {

// ----------------------------------------------------------------------
// Component construction and destruction
// ----------------------------------------------------------------------

CfsTlmStripper ::CfsTlmStripper(const char* const compName) : CfsTlmStripperComponentBase(compName) {}

CfsTlmStripper ::~CfsTlmStripper() {}

// ----------------------------------------------------------------------
// Handler implementations for typed input ports
// ----------------------------------------------------------------------

void CfsTlmStripper ::dataIn_handler(FwIndexType portNum, Fw::Buffer& data, const ComCfg::FrameContext& context) {
    const U8* const bytes = data.getData();
    const FwSizeType size = data.getSize();
    if (size < CFS_TLM_STRIPPER_PRI_HDR_SIZE) {
        this->log_WARNING_HI_MalformedPacket(static_cast<U32>(size));
        this->dataReturnOut_out(0, data, context);
        return;
    }

    // Telemetry space packets carrying a cFS secondary header have it stripped; all
    // other packets (commands, packets without the secondary header flag) pass
    // through unchanged. The output is always an allocated copy so the incoming
    // buffer can be returned to its sender synchronously.
    const bool strip = ((bytes[0] & CFS_TLM_STRIPPER_TYPE_FLAG) == 0) &&
                       ((bytes[0] & CFS_TLM_STRIPPER_SEC_HDR_FLAG) != 0) &&
                       (size >= (CFS_TLM_STRIPPER_PRI_HDR_SIZE + CFS_TLM_STRIPPER_SEC_HDR_SIZE + 1));
    const FwSizeType outSize = strip ? (size - CFS_TLM_STRIPPER_SEC_HDR_SIZE) : size;

    Fw::Buffer out = this->bufferAllocate_out(0, static_cast<Fw::Buffer::SizeType>(outSize));
    if (out.getSize() < outSize) {
        this->log_WARNING_HI_AllocationFailed(static_cast<U32>(outSize));
        if (out.getSize() > 0) {
            this->bufferDeallocate_out(0, out);
        }
        this->dataReturnOut_out(0, data, context);
        return;
    }

    U8* const outBytes = out.getData();
    ComCfg::FrameContext outContext = context;
    if (strip) {
        (void)std::memcpy(outBytes, bytes, CFS_TLM_STRIPPER_PRI_HDR_SIZE);
        (void)std::memcpy(&outBytes[CFS_TLM_STRIPPER_PRI_HDR_SIZE],
                          &bytes[CFS_TLM_STRIPPER_PRI_HDR_SIZE + CFS_TLM_STRIPPER_SEC_HDR_SIZE],
                          size - CFS_TLM_STRIPPER_PRI_HDR_SIZE - CFS_TLM_STRIPPER_SEC_HDR_SIZE);
        // Clear the secondary header flag and shrink the packet data length field
        outBytes[0] &= static_cast<U8>(~CFS_TLM_STRIPPER_SEC_HDR_FLAG);
        const FwSizeType lengthToken =
            ((static_cast<FwSizeType>(bytes[CFS_TLM_STRIPPER_LENGTH_OFFSET]) << 8) |
             static_cast<FwSizeType>(bytes[CFS_TLM_STRIPPER_LENGTH_OFFSET + 1])) -
            CFS_TLM_STRIPPER_SEC_HDR_SIZE;
        outBytes[CFS_TLM_STRIPPER_LENGTH_OFFSET] = static_cast<U8>((lengthToken >> 8) & 0xFF);
        outBytes[CFS_TLM_STRIPPER_LENGTH_OFFSET + 1] = static_cast<U8>(lengthToken & 0xFF);
        outContext.set_hasSecHdr(false);
    } else {
        (void)std::memcpy(outBytes, bytes, size);
    }
    out.setSize(static_cast<Fw::Buffer::SizeType>(outSize));

    this->dataOut_out(0, out, outContext);
    this->dataReturnOut_out(0, data, context);  // return ownership of the original data buffer
}

void CfsTlmStripper ::comStatusIn_handler(FwIndexType portNum, Fw::Success& condition) {
    if (this->isConnected_comStatusOut_OutputPort(portNum)) {
        this->comStatusOut_out(portNum, condition);
    }
}

void CfsTlmStripper ::dataReturnIn_handler(FwIndexType portNum,
                                           Fw::Buffer& frameBuffer,
                                           const ComCfg::FrameContext& context) {
    // dataReturnIn is the allocated buffer coming back from the dataOut port
    this->bufferDeallocate_out(0, frameBuffer);
}

}  // namespace FPrimeCfs
