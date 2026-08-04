// ======================================================================
// \title  CfsTlmFramer.cpp
// \brief  cpp file for CfsTlmFramer component implementation class
// ======================================================================

#include "FPrimeCfs/CfsTlmFramer/CfsTlmFramer.hpp"
#include <cstring>
#include "Fw/FPrimeBasicTypes.hpp"
#include "Fw/Logger/Logger.hpp"

namespace FPrimeCfs {

// ----------------------------------------------------------------------
// Component construction and destruction
// ----------------------------------------------------------------------

CfsTlmFramer ::CfsTlmFramer(const char* const compName) : CfsTlmFramerComponentBase(compName) {}

CfsTlmFramer ::~CfsTlmFramer() {}

// ----------------------------------------------------------------------
// Handler implementations for typed input ports
// ----------------------------------------------------------------------

void CfsTlmFramer ::dataIn_handler(FwIndexType portNum, Fw::Buffer& data, const ComCfg::FrameContext& context) {
    const U8* const buffer_data = data.getData();
    const FwSizeType buffer_size = data.getSize();

    // First pass: validate the packet structure and count telemetry packets needing a secondary header
    FwSizeType frame_count = 0;
    FwSizeType offset = 0;
    bool well_formed = true;
    while (offset < buffer_size) {
        if ((buffer_size - offset) < CFS_TLM_FRAMER_SPACE_PACKET_HEADER_SIZE) {
            well_formed = false;
            break;
        }
        const FwSizeType length_token =
            (static_cast<FwSizeType>(buffer_data[offset + CFS_TLM_FRAMER_SPACE_PACKET_LENGTH_OFFSET]) << 8) |
            static_cast<FwSizeType>(buffer_data[offset + CFS_TLM_FRAMER_SPACE_PACKET_LENGTH_OFFSET + 1]);
        const FwSizeType packet_size = CFS_TLM_FRAMER_SPACE_PACKET_HEADER_SIZE + length_token + 1;
        if (packet_size > (buffer_size - offset)) {
            well_formed = false;
            break;
        }
        const U8 streamIdHigh = buffer_data[offset];
        const bool isCommand = (streamIdHigh & static_cast<U8>(CFS_TLM_FRAMER_SPACE_PACKET_TYPE_MASK >> 8)) != 0;
        const bool hasSecHdr = (streamIdHigh & static_cast<U8>(CFS_TLM_FRAMER_SPACE_PACKET_SEC_HDR_MASK >> 8)) != 0;
        if ((not isCommand) and (not hasSecHdr)) {
            frame_count++;
        }
        offset += packet_size;
    }
    if (not well_formed) {
        // Malformed buffers are forwarded verbatim; the downstream consumer performs its own validation
        Fw::Logger::log("[WARNING] Malformed space packet buffer of %" PRI_FwSizeType
                        " bytes forwarded without cFS telemetry framing\n",
                        buffer_size);
        frame_count = 0;
    }

    // Allocate the output buffer: one telemetry secondary header per framed packet
    const FwSizeType framed_size = buffer_size + (frame_count * CFS_TLM_FRAMER_TLM_SEC_HDR_SIZE);
    Fw::Buffer framed = this->bufferAllocate_out(0, static_cast<Fw::Buffer::SizeType>(framed_size));
    if (framed.getSize() < framed_size) {
        Fw::Logger::log("[ERROR] Failed to allocate %" PRI_FwSizeType " byte buffer for cFS telemetry framing\n",
                        framed_size);
        if (framed.getSize() > 0) {
            this->bufferDeallocate_out(0, framed);
        }
        // Keep the com queue flowing despite the dropped buffer
        Fw::Success success = Fw::Success::SUCCESS;
        this->comStatusOut_out(0, success);
        this->dataReturnOut_out(0, data, context);
        return;
    }

    // Second pass: copy each packet, inserting the telemetry secondary header where counted
    U8* const framed_data = framed.getData();
    FwSizeType write_offset = 0;
    offset = 0;
    while (offset < buffer_size) {
        FwSizeType packet_size = buffer_size - offset;
        bool frame_packet = false;
        if (well_formed) {
            const FwSizeType length_token =
                (static_cast<FwSizeType>(buffer_data[offset + CFS_TLM_FRAMER_SPACE_PACKET_LENGTH_OFFSET]) << 8) |
                static_cast<FwSizeType>(buffer_data[offset + CFS_TLM_FRAMER_SPACE_PACKET_LENGTH_OFFSET + 1]);
            packet_size = CFS_TLM_FRAMER_SPACE_PACKET_HEADER_SIZE + length_token + 1;
            const U8 streamIdHigh = buffer_data[offset];
            const bool isCommand = (streamIdHigh & static_cast<U8>(CFS_TLM_FRAMER_SPACE_PACKET_TYPE_MASK >> 8)) != 0;
            const bool hasSecHdr = (streamIdHigh & static_cast<U8>(CFS_TLM_FRAMER_SPACE_PACKET_SEC_HDR_MASK >> 8)) != 0;
            frame_packet = (not isCommand) and (not hasSecHdr);
        }
        if (frame_packet) {
            // Copy and rewrite the primary header: set the secondary header flag and extend the length
            (void)std::memcpy(&framed_data[write_offset], &buffer_data[offset],
                              CFS_TLM_FRAMER_SPACE_PACKET_HEADER_SIZE);
            framed_data[write_offset] |= static_cast<U8>(CFS_TLM_FRAMER_SPACE_PACKET_SEC_HDR_MASK >> 8);
            const FwSizeType new_length_token =
                ((static_cast<FwSizeType>(buffer_data[offset + CFS_TLM_FRAMER_SPACE_PACKET_LENGTH_OFFSET]) << 8) |
                 static_cast<FwSizeType>(buffer_data[offset + CFS_TLM_FRAMER_SPACE_PACKET_LENGTH_OFFSET + 1])) +
                CFS_TLM_FRAMER_TLM_SEC_HDR_SIZE;
            framed_data[write_offset + CFS_TLM_FRAMER_SPACE_PACKET_LENGTH_OFFSET] =
                static_cast<U8>((new_length_token >> 8) & 0xFF);
            framed_data[write_offset + CFS_TLM_FRAMER_SPACE_PACKET_LENGTH_OFFSET + 1] =
                static_cast<U8>(new_length_token & 0xFF);
            // Insert the telemetry secondary header followed by the unmodified payload
            this->writeTelemetrySecondaryHeader(&framed_data[write_offset + CFS_TLM_FRAMER_SPACE_PACKET_HEADER_SIZE]);
            (void)std::memcpy(
                &framed_data[write_offset + CFS_TLM_FRAMER_SPACE_PACKET_HEADER_SIZE + CFS_TLM_FRAMER_TLM_SEC_HDR_SIZE],
                &buffer_data[offset + CFS_TLM_FRAMER_SPACE_PACKET_HEADER_SIZE],
                packet_size - CFS_TLM_FRAMER_SPACE_PACKET_HEADER_SIZE);
            write_offset += packet_size + CFS_TLM_FRAMER_TLM_SEC_HDR_SIZE;
        } else {
            (void)std::memcpy(&framed_data[write_offset], &buffer_data[offset], packet_size);
            write_offset += packet_size;
        }
        offset += packet_size;
    }
    FW_ASSERT(write_offset == framed_size, static_cast<FwAssertArgType>(write_offset),
              static_cast<FwAssertArgType>(framed_size));
    framed.setSize(static_cast<Fw::Buffer::SizeType>(framed_size));

    this->dataOut_out(0, framed, context);
    this->dataReturnOut_out(0, data, context);  // return ownership of the original data buffer
}

void CfsTlmFramer ::dataReturnIn_handler(FwIndexType portNum, Fw::Buffer& data, const ComCfg::FrameContext& context) {
    // dataReturnIn is the allocated buffer coming back from the dataOut port
    this->bufferDeallocate_out(0, data);
}

void CfsTlmFramer ::comStatusIn_handler(FwIndexType portNum, Fw::Success& condition) {
    if (this->isConnected_comStatusOut_OutputPort(portNum)) {
        this->comStatusOut_out(portNum, condition);
    }
}

// ----------------------------------------------------------------------
// Helpers
// ----------------------------------------------------------------------

void CfsTlmFramer ::writeTelemetrySecondaryHeader(U8* location) {
    const Fw::Time now = this->getTime();
    const U32 seconds = now.getSeconds();
    // Convert microseconds to 1/65536-second subseconds, per cFS telemetry time conventions
    const U64 subseconds64 = (static_cast<U64>(now.getUSeconds()) << 16) / CFS_TLM_FRAMER_USECS_PER_SECOND;
    const U16 subseconds = static_cast<U16>(subseconds64 & 0xFFFF);
    location[0] = static_cast<U8>((seconds >> 24) & 0xFF);
    location[1] = static_cast<U8>((seconds >> 16) & 0xFF);
    location[2] = static_cast<U8>((seconds >> 8) & 0xFF);
    location[3] = static_cast<U8>(seconds & 0xFF);
    location[4] = static_cast<U8>((subseconds >> 8) & 0xFF);
    location[5] = static_cast<U8>(subseconds & 0xFF);
}

}  // namespace FPrimeCfs
