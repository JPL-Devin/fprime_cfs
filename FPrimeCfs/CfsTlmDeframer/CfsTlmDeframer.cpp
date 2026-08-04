// ======================================================================
// \title  CfsTlmDeframer.cpp
// \brief  cpp file for CfsTlmDeframer component implementation class
// ======================================================================

#include "FPrimeCfs/CfsTlmDeframer/CfsTlmDeframer.hpp"
#include <cstring>
#include "Fw/FPrimeBasicTypes.hpp"
#include "Fw/Logger/Logger.hpp"

namespace FPrimeCfs {

// ----------------------------------------------------------------------
// Component construction and destruction
// ----------------------------------------------------------------------

CfsTlmDeframer ::CfsTlmDeframer(const char* const compName) : CfsTlmDeframerComponentBase(compName) {}

CfsTlmDeframer ::~CfsTlmDeframer() {}

// ----------------------------------------------------------------------
// Handler implementations for typed input ports
// ----------------------------------------------------------------------

void CfsTlmDeframer ::dataIn_handler(FwIndexType portNum, Fw::Buffer& data, const ComCfg::FrameContext& context) {
    U8* const buffer_data = data.getData();
    const FwSizeType buffer_size = data.getSize();

    bool strip = false;
    if (buffer_size >= (CFS_TLM_DEFRAMER_SPACE_PACKET_HEADER_SIZE + CFS_TLM_DEFRAMER_TLM_SEC_HDR_SIZE)) {
        const U8 streamIdHigh = buffer_data[0];
        const bool isCommand = (streamIdHigh & static_cast<U8>(CFS_TLM_DEFRAMER_SPACE_PACKET_TYPE_MASK >> 8)) != 0;
        const bool hasSecHdr = (streamIdHigh & static_cast<U8>(CFS_TLM_DEFRAMER_SPACE_PACKET_SEC_HDR_MASK >> 8)) != 0;
        const FwSizeType length_token =
            (static_cast<FwSizeType>(buffer_data[CFS_TLM_DEFRAMER_SPACE_PACKET_LENGTH_OFFSET]) << 8) |
            static_cast<FwSizeType>(buffer_data[CFS_TLM_DEFRAMER_SPACE_PACKET_LENGTH_OFFSET + 1]);
        const FwSizeType packet_size = CFS_TLM_DEFRAMER_SPACE_PACKET_HEADER_SIZE + length_token + 1;
        // Strip only well-formed telemetry packets carrying a secondary header
        strip = (not isCommand) and hasSecHdr and (packet_size <= buffer_size) and
                (length_token + 1 > CFS_TLM_DEFRAMER_TLM_SEC_HDR_SIZE);
        if ((not isCommand) and hasSecHdr and (not strip)) {
            Fw::Logger::log("[WARNING] Malformed %" PRI_FwSizeType
                            " byte telemetry packet forwarded with its secondary header\n",
                            buffer_size);
        }
    }

    if (strip) {
        // Rewrite the primary header: clear the secondary header flag and shorten the length
        buffer_data[0] &= static_cast<U8>(~(CFS_TLM_DEFRAMER_SPACE_PACKET_SEC_HDR_MASK >> 8));
        const FwSizeType length_token =
            (static_cast<FwSizeType>(buffer_data[CFS_TLM_DEFRAMER_SPACE_PACKET_LENGTH_OFFSET]) << 8) |
            static_cast<FwSizeType>(buffer_data[CFS_TLM_DEFRAMER_SPACE_PACKET_LENGTH_OFFSET + 1]);
        const FwSizeType new_length_token = length_token - CFS_TLM_DEFRAMER_TLM_SEC_HDR_SIZE;
        buffer_data[CFS_TLM_DEFRAMER_SPACE_PACKET_LENGTH_OFFSET] = static_cast<U8>((new_length_token >> 8) & 0xFF);
        buffer_data[CFS_TLM_DEFRAMER_SPACE_PACKET_LENGTH_OFFSET + 1] = static_cast<U8>(new_length_token & 0xFF);
        // Remove the telemetry secondary header by shifting the payload forward in place
        const FwSizeType payload_offset =
            CFS_TLM_DEFRAMER_SPACE_PACKET_HEADER_SIZE + CFS_TLM_DEFRAMER_TLM_SEC_HDR_SIZE;
        (void)std::memmove(&buffer_data[CFS_TLM_DEFRAMER_SPACE_PACKET_HEADER_SIZE], &buffer_data[payload_offset],
                           data.getSize() - payload_offset);
        data.setSize(static_cast<Fw::Buffer::SizeType>(data.getSize() - CFS_TLM_DEFRAMER_TLM_SEC_HDR_SIZE));
    }
    this->dataOut_out(0, data, context);
}

void CfsTlmDeframer ::dataReturnIn_handler(FwIndexType portNum, Fw::Buffer& data, const ComCfg::FrameContext& context) {
    // Return ownership of the buffer to the upstream sender
    this->dataReturnOut_out(0, data, context);
}

}  // namespace FPrimeCfs
