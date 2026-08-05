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
    U8* const bytes = data.getData();
    const FwSizeType size = data.getSize();
    if (size < CFS_TLM_STRIPPER_PRI_HDR_SIZE) {
        this->log_WARNING_HI_MalformedPacket(static_cast<U32>(size));
        this->dataReturnOut_out(0, data, context);
        return;
    }

    // Strip telemetry packets with a secondary header in place: rebuild the primary
    // header just ahead of the payload and advance the data pointer; others pass through
    const bool strip = ((bytes[0] & CFS_TLM_STRIPPER_TYPE_FLAG) == 0) &&
                       ((bytes[0] & CFS_TLM_STRIPPER_SEC_HDR_FLAG) != 0) &&
                       (size >= (CFS_TLM_STRIPPER_PRI_HDR_SIZE + CFS_TLM_STRIPPER_SEC_HDR_SIZE + 1));

    ComCfg::FrameContext outContext = context;
    if (strip) {
        const FwSizeType lengthToken =
            ((static_cast<FwSizeType>(bytes[CFS_TLM_STRIPPER_LENGTH_OFFSET]) << 8) |
             static_cast<FwSizeType>(bytes[CFS_TLM_STRIPPER_LENGTH_OFFSET + 1])) -
            CFS_TLM_STRIPPER_SEC_HDR_SIZE;
        U8* const outBytes = &bytes[CFS_TLM_STRIPPER_SEC_HDR_SIZE];
        (void)std::memmove(outBytes, bytes, CFS_TLM_STRIPPER_PRI_HDR_SIZE);
        // Clear the secondary header flag and shrink the packet data length field
        outBytes[0] &= static_cast<U8>(~CFS_TLM_STRIPPER_SEC_HDR_FLAG);
        outBytes[CFS_TLM_STRIPPER_LENGTH_OFFSET] = static_cast<U8>((lengthToken >> 8) & 0xFF);
        outBytes[CFS_TLM_STRIPPER_LENGTH_OFFSET + 1] = static_cast<U8>(lengthToken & 0xFF);
        data.setData(outBytes);
        data.setSize(static_cast<Fw::Buffer::SizeType>(size - CFS_TLM_STRIPPER_SEC_HDR_SIZE));
        outContext.set_hasSecHdr(false);
    }

    this->dataOut_out(0, data, outContext);
}

void CfsTlmStripper ::comStatusIn_handler(FwIndexType portNum, Fw::Success& condition) {
    if (this->isConnected_comStatusOut_OutputPort(portNum)) {
        this->comStatusOut_out(portNum, condition);
    }
}

void CfsTlmStripper ::dataReturnIn_handler(FwIndexType portNum,
                                           Fw::Buffer& frameBuffer,
                                           const ComCfg::FrameContext& context) {
    // Return the buffer to the original sender; allocator-owned buffers are tracked
    // by buffer context, so the advanced data pointer does not affect their return
    this->dataReturnOut_out(0, frameBuffer, context);
}

}  // namespace FPrimeCfs
