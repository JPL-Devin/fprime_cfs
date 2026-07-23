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

CFE_Status_t CfsBridge ::configure(const FwSizeType pipeDepth, const char* pipeName, const bool paused) {
    CFE_Status_t status = CFE_SB_CreatePipe(&this->inputPipe, static_cast<uint16>(pipeDepth), pipeName);
    if (status == CFE_SUCCESS) {
        this->m_configurationState = CONFIGURED;
    }
    this->m_flowControlled = paused;
    return status;
}

Fw::QueuedComponentBase::MsgDispatchStatus CfsBridge ::process() {
    // First drain the component's message queue. This ensures messages have been returned etc
    Fw::QueuedComponentBase::MsgDispatchStatus status = this->dispatchCurrentMessages();
    // Next, poll as long as we are not in an exit condition
    if (status != Fw::QueuedComponentBase::MSG_DISPATCH_EXIT && this->m_configurationState == SUBSCRIBED) {
        // Preroll the com pipeline to allow downlink
        if (not this->m_prerolled and this->isConnected_comStatusOut_OutputPort(0)) {
            Fw::Success status = Fw::Success::SUCCESS;
            this->comStatusOut_out(0, status);
            this->m_prerolled = true;
        }

        this->poll();
    }
    return status;
}

CFE_Status_t CfsBridge ::subscribe(const ComCfg::Apid::T apid) {
    FW_ASSERT(this->m_configurationState != UNCONFIGURED);
    CFE_SB_MsgId_t msgId = this->getCfsMessageId(apid);
    CFE_Status_t status = CFE_SB_Subscribe(msgId, this->inputPipe);
    if (status == CFE_SUCCESS) {
        this->m_configurationState = SUBSCRIBED;
        Fw::Logger::log("[INFO] Successfully subscribed to message ID: 0x%08x\n", msgId.Value);
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
    CFE_Status_t status = CFE_SB_Subscribe(msgId, this->inputPipe);
    if (status == CFE_SUCCESS) {
        this->m_configurationState = SUBSCRIBED;
        Fw::Logger::log("[INFO] Successfully subscribed to message ID: 0x%08x\n", message_id);
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
    CFE_Status_t status = CFE_SB_ReceiveBuffer(&buffer, this->inputPipe, CFE_SB_POLL);
    if (status == CFE_SUCCESS) {
        Fw::Logger::log("[DEBUG] Received message!\n");
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
    while ((buffer_size - offset) >= CFS_BRIDGE_SPACE_PACKET_HEADER_SIZE) {
        // CCSDS packet data length field is the number of payload bytes minus one
        const FwSizeType packet_size = CFS_BRIDGE_SPACE_PACKET_HEADER_SIZE + 1 +
            ((static_cast<FwSizeType>(buffer_data[offset + 4]) << 8) | static_cast<FwSizeType>(buffer_data[offset + 5]));
        if (packet_size > (buffer_size - offset)) {
            Fw::Logger::log("[ERROR] Dropping truncated space packet: %" PRI_FwSizeType " bytes needed, %" PRI_FwSizeType " available\n",
                            packet_size, buffer_size - offset);
            break;
        }
        CFE_MSG_Message_t* message_pointer = reinterpret_cast<CFE_MSG_Message_t*>(&buffer_data[offset]);
        CFE_Status_t status = CFE_SB_TransmitMsg(message_pointer, false);
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

void CfsBridge ::dataReturnIn_handler(FwIndexType portNum, Fw::Buffer &data, const ComCfg::FrameContext &context)
{
    // cFS does not return messages explicitly
}

void CfsBridge ::comStatusIn_handler(FwIndexType portNum, Fw::Success &status)
{
    this->m_paused = (status == Fw::Success::SUCCESS) ? false : true;
}

} // namespace FPrimeCfs
