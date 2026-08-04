// ======================================================================
// \title  CfsBridge.cpp
// \author mstarch
// \brief  cpp file for CfsBridge component implementation class
// ======================================================================

#include "FPrimeCfs/CfsBridge/CfsBridge.hpp"
#include "Fw/Logger/Logger.hpp"
#include <cstring>
#include <limits>

// Some cFE versions (e.g. draco) use compound literals in inline functions,
// which is a GCC extension when compiled as C++
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
extern "C" {
    #include "cfe.h"
    #include "cfe_config.h"
    #include "cfe_sb.h"   // for CFE_SB_TransmitMsg
    #include "fprime_cfs_compatibility.h"
}
#pragma GCC diagnostic pop

namespace FPrimeCfs
{

// ----------------------------------------------------------------------
// Component construction and destruction
// ----------------------------------------------------------------------

CfsBridge ::CfsBridge(const char *const compName) : CfsBridgeComponentBase(compName) {}

CfsBridge ::~CfsBridge() {}

CFE_Status_t CfsBridge ::configure(const FwSizeType pipeDepth,
                                   const char* pipeName,
                                   const bool paused,
                                   const bool wrapFprimeCommands) {
    // Pipe depth is a uint16 in cFE; reject depths that would silently truncate
    if (pipeDepth > std::numeric_limits<uint16>::max()) {
        return CFE_SB_BAD_ARGUMENT;
    }
    CFE_Status_t status = CFE_SB_CreatePipe(&this->m_inputPipe, static_cast<uint16>(pipeDepth), pipeName);
    if (status == CFE_SUCCESS) {
        this->m_configurationState = CONFIGURED;
    }
    this->m_flowControlled = paused;
    this->m_wrapFprimeCommands = wrapFprimeCommands;
    return status;
}

Fw::QueuedComponentBase::MsgDispatchStatus CfsBridge ::process() {
    // First drain the component's message queue. This ensures messages have been returned etc
    Fw::QueuedComponentBase::MsgDispatchStatus status = this->dispatchCurrentMessages();
    // Next, poll as long as we are not in an exit condition
    if (status != Fw::QueuedComponentBase::MSG_DISPATCH_EXIT && this->m_configurationState == SUBSCRIBED) {
        // Preroll the com pipeline to allow downlink
        if (not this->m_prerolled and this->isConnected_comStatusOut_OutputPort(0)) {
            Fw::Success comStatus = Fw::Success::SUCCESS;
            this->comStatusOut_out(0, comStatus);
            this->m_prerolled = true;
        }

        this->poll();
    }
    return status;
}

CFE_Status_t CfsBridge ::subscribe(const ComCfg::Apid::T apid) {
    FW_ASSERT(this->m_configurationState != UNCONFIGURED);
    CFE_SB_MsgId_t msgId = this->getCfsMessageId(apid);
    CFE_Status_t status = CFE_SB_Subscribe(msgId, this->m_inputPipe);
    if (status == CFE_SUCCESS) {
        this->m_configurationState = SUBSCRIBED;
    }
    return status;
}

CFE_Status_t CfsBridge ::subscribeCfs(const ComCfg::Apid::T apid, const CfsMessageType type) {
    FW_ASSERT(this->m_configurationState != UNCONFIGURED);
    // cFS messages carry a secondary header: the stream identifier (and thus the message ID) has
    // the secondary header flag set, and the packet type bit set for commands and clear for telemetry
    U32 message_id = (static_cast<U32>(apid) & CFS_BRIDGE_SPACE_PACKET_APID_MASK) |
                     CFS_BRIDGE_SPACE_PACKET_SEC_HDR_MASK |
                     ((type == CfsMessageType::COMMAND) ? CFS_BRIDGE_SPACE_PACKET_TYPE_MASK : 0);
    CFE_SB_MsgId_t msgId = CFE_SB_ValueToMsgId(message_id);
    CFE_Status_t status = CFE_SB_Subscribe(msgId, this->m_inputPipe);
    if (status == CFE_SUCCESS) {
        this->m_configurationState = SUBSCRIBED;
    }
    return status;
}

// ----------------------------------------------------------------------
// Handler implementations for typed input ports
// ----------------------------------------------------------------------

CFE_SB_MsgId_t CfsBridge ::getCfsMessageId(const ComCfg::Apid::T apid) {
    // Messages on the software bus are complete CCSDS space packets, so message IDs mirror the
    // space packet stream identifier: PVN 0, secondary header flag 0, APID in the low 11 bits, and
    // the packet type bit set for commands (uplink) and clear for telemetry (downlink)
    U32 message_id = static_cast<U32>(apid) & CFS_BRIDGE_SPACE_PACKET_APID_MASK;
    if (apid == ComCfg::Apid::FW_PACKET_COMMAND) {
        message_id |= CFS_BRIDGE_SPACE_PACKET_TYPE_MASK;
    }
    return CFE_SB_ValueToMsgId(message_id);
}

void CfsBridge ::poll() {
    if (this->m_flowControlled and this->m_paused) {
        return;
    }

    CFE_SB_Buffer_t* buffer = nullptr;
    CFE_Status_t status = CFE_SB_ReceiveBuffer(&buffer, this->m_inputPipe, CFE_SB_POLL);
    if (status == CFE_SUCCESS) {
        CFE_MSG_Message_t* received_message = &buffer->Msg;

        // Software bus messages are external input: drop with an error, not an assert
        CFE_MSG_Size_t message_size = 0;
        status = CFE_MSG_GetSize(received_message, &message_size);
        if (status != CFE_SUCCESS) {
            Fw::Logger::log("[ERROR] Failed to get size from received message: 0x%08x\n", status);
            return;
        }

        // Forward the complete message (a CCSDS space packet) with a default context; a downstream
        // deframer (e.g. Svc.Ccsds.SpacePacketDeframer) derives the APID and other fields from the headers
        Fw::Buffer fwBuffer(reinterpret_cast<U8*>(received_message), static_cast<FwSizeType>(message_size));
        ComCfg::FrameContext context;
        this->m_paused = true;
        // Send the message out of this port
        this->dataOut_out(0, fwBuffer, context);
    } else if (status != CFE_SB_NO_MESSAGE) {
        Fw::Logger::log("[ERROR] Error receiving message from cFS pipe: 0x%08x\n", status);
    }
}

void CfsBridge ::dataIn_handler(FwIndexType portNum, Fw::Buffer &data, const ComCfg::FrameContext &context)
{
    // The incoming buffer contains one or more complete CCSDS space packets (e.g. from a space packet
    // framer, or several concatenated by an aggregator). Each packet is transmitted on the software bus
    // as its own message; the message ID is derived from the packet's stream identifier by cFS.
    U8* const buffer_data = data.getData();
    const FwSizeType buffer_size = data.getSize();
    FwSizeType offset = 0;
    // Each complete packet is at least header + 1 payload byte, bounding the packet count
    const FwSizeType max_packets = buffer_size / (CFS_BRIDGE_SPACE_PACKET_HEADER_SIZE + 1);
    for (FwSizeType packet_count = 0;
         (packet_count < max_packets) && ((buffer_size - offset) >= CFS_BRIDGE_SPACE_PACKET_HEADER_SIZE);
         packet_count++) {
        // CCSDS packet data length field is the number of payload bytes minus one
        const FwSizeType packet_size = CFS_BRIDGE_SPACE_PACKET_HEADER_SIZE + 1 +
            ((static_cast<FwSizeType>(buffer_data[offset + CFS_BRIDGE_SPACE_PACKET_LENGTH_OFFSET]) << 8) |
             static_cast<FwSizeType>(buffer_data[offset + CFS_BRIDGE_SPACE_PACKET_LENGTH_OFFSET + 1]));
        if (packet_size > (buffer_size - offset)) {
            Fw::Logger::log("[ERROR] Dropping truncated space packet: %" PRI_FwSizeType " bytes needed, %" PRI_FwSizeType " available\n",
                            packet_size, buffer_size - offset);
            break;
        }
        // The stream identifier's high byte carries the packet type bit (0x10) and secondary header flag (0x08)
        const U8 streamIdHigh = buffer_data[offset];
        const bool isCommand = (streamIdHigh & static_cast<U8>(CFS_BRIDGE_SPACE_PACKET_TYPE_MASK >> 8)) != 0;
        const bool hasSecHdr = (streamIdHigh & static_cast<U8>(CFS_BRIDGE_SPACE_PACKET_SEC_HDR_MASK >> 8)) != 0;
        if (this->m_wrapFprimeCommands and isCommand and (not hasSecHdr) and
            ((packet_size + CFS_BRIDGE_CMD_SEC_HDR_SIZE) > CFS_BRIDGE_MAX_WRAPPED_PACKET_SIZE)) {
            Fw::Logger::log("[ERROR] Cannot wrap %" PRI_FwSizeType " byte packet as a cFS command packet\n",
                            packet_size);
            offset += packet_size;
            continue;
        }
        CFE_Status_t status;
        if (this->m_wrapFprimeCommands and isCommand and (not hasSecHdr)) {
            // F Prime command space packet: wrap as a valid cFS command packet before transmission
            status = this->transmitWrappedCommand(&buffer_data[offset], packet_size);
        } else {
            // Cast justified: software bus messages are complete CCSDS space packets and the cFS API takes them
            // as CFE_MSG_Message_t
            CFE_MSG_Message_t* message_pointer = reinterpret_cast<CFE_MSG_Message_t*>(&buffer_data[offset]);
            status = CFE_SB_TransmitMsg(message_pointer, false);
        }
        if (status != CFE_SUCCESS) {
            Fw::Logger::log("[ERROR] Failed to transmit message to software bus: 0x%08x\n", status);
        }
        offset += packet_size;
    }
    if (offset != buffer_size) {
        Fw::Logger::log("[ERROR] Dropping %" PRI_FwSizeType " residual bytes not forming a complete space packet\n",
                        buffer_size - offset);
    }
    // Ownership of the data is always returned to the sender, and com status always reports success as the software
    // bus does not support retries
    this->dataReturnOut_out(0, data, context);
    if (this->isConnected_comStatusOut_OutputPort(0)) {
        Fw::Success comStatus = Fw::Success::SUCCESS;
        this->comStatusOut_out(0, comStatus);
    }
}

CFE_Status_t CfsBridge ::transmitWrappedCommand(const U8* packet, const FwSizeType size) {
    const FwSizeType wrappedSize = size + CFS_BRIDGE_CMD_SEC_HDR_SIZE;
    // The caller validates sizes; this guard is defensive against future callers
    if ((size < CFS_BRIDGE_SPACE_PACKET_HEADER_SIZE) or (wrappedSize > CFS_BRIDGE_MAX_WRAPPED_PACKET_SIZE)) {
        return CFE_SB_BAD_ARGUMENT;
    }
    U8* const wrapped = this->m_wrapStorage;
    (void)std::memcpy(wrapped, packet, CFS_BRIDGE_SPACE_PACKET_HEADER_SIZE);
    // Set the secondary header flag in the stream identifier
    wrapped[0] |= static_cast<U8>(CFS_BRIDGE_SPACE_PACKET_SEC_HDR_MASK >> 8);
    // The length field grows by the inserted secondary header size
    const FwSizeType lengthToken =
        ((static_cast<FwSizeType>(packet[CFS_BRIDGE_SPACE_PACKET_LENGTH_OFFSET]) << 8) |
         static_cast<FwSizeType>(packet[CFS_BRIDGE_SPACE_PACKET_LENGTH_OFFSET + 1])) +
        CFS_BRIDGE_CMD_SEC_HDR_SIZE;
    wrapped[CFS_BRIDGE_SPACE_PACKET_LENGTH_OFFSET] = static_cast<U8>((lengthToken >> 8) & 0xFF);
    wrapped[CFS_BRIDGE_SPACE_PACKET_LENGTH_OFFSET + 1] = static_cast<U8>(lengthToken & 0xFF);
    // cFS command secondary header: function code, then checksum (computed below over the whole packet)
    wrapped[CFS_BRIDGE_SPACE_PACKET_HEADER_SIZE] = CFS_BRIDGE_FPRIME_COMMAND_FUNCTION_CODE;
    wrapped[CFS_BRIDGE_SPACE_PACKET_HEADER_SIZE + 1] = 0;
    (void)std::memcpy(&wrapped[CFS_BRIDGE_SPACE_PACKET_HEADER_SIZE + CFS_BRIDGE_CMD_SEC_HDR_SIZE],
                      &packet[CFS_BRIDGE_SPACE_PACKET_HEADER_SIZE], size - CFS_BRIDGE_SPACE_PACKET_HEADER_SIZE);
    // Checksum per CFE_MSG conventions: XOR of every packet byte with 0xFF must equal zero
    U8 checksum = 0xFF;
    for (FwSizeType i = 0; i < wrappedSize; i++) {
        checksum ^= wrapped[i];
    }
    wrapped[CFS_BRIDGE_SPACE_PACKET_HEADER_SIZE + 1] = checksum;
    // Cast justified: m_wrapStorage is alignas(CFE_MSG_Message_t) and holds a complete space packet
    return CFE_SB_TransmitMsg(reinterpret_cast<CFE_MSG_Message_t*>(wrapped), false);
}

void CfsBridge ::dataReturnIn_handler(FwIndexType portNum, Fw::Buffer &data, const ComCfg::FrameContext &context)
{
    // cFS does not return messages explicitly
}

void CfsBridge ::schedIn_handler(FwIndexType portNum, U32 context)
{
    (void)this->process();
}

void CfsBridge ::comStatusIn_handler(FwIndexType portNum, Fw::Success &status)
{
    this->m_paused = (status == Fw::Success::SUCCESS) ? false : true;
}

} // namespace FPrimeCfs
