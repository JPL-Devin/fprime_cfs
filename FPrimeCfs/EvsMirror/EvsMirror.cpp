// ======================================================================
// \title  EvsMirror.cpp
// \brief  cpp file for EvsMirror component implementation class
// ======================================================================

#include "FPrimeCfs/EvsMirror/EvsMirror.hpp"
#include "Fw/Logger/Logger.hpp"

static_assert(FW_ENABLE_TEXT_LOGGING, "EvsMirror requires text logging (FW_ENABLE_TEXT_LOGGING) to receive event text");

namespace FPrimeCfs {

// ----------------------------------------------------------------------
// Component construction and destruction
// ----------------------------------------------------------------------

EvsMirror ::EvsMirror(const char* const compName)
    : EvsMirrorComponentBase(compName), m_appId(CFE_ES_APPID_UNDEFINED) {}

EvsMirror ::~EvsMirror() {}

void EvsMirror ::init(FwEnumStoreType instance) {
    EvsMirrorComponentBase::init(instance);
    // Capture the host application ID while running on the app's main
    // (ES-registered) task; see the declaration comment for the rationale
    CFE_Status_t status = CFE_ES_GetAppID(&this->m_appId);
    if (status != CFE_SUCCESS) {
        this->m_appId = CFE_ES_APPID_UNDEFINED;
        Fw::Logger::log("[ERROR] EvsMirror failed to get cFS app ID: 0x%08x\n", static_cast<U32>(status));
    }
}

// ----------------------------------------------------------------------
// Handler implementations for typed input ports
// ----------------------------------------------------------------------

void EvsMirror ::TextLogger_handler(FwIndexType portNum,
                                    FwEventIdType id,
                                    Fw::Time& timeTag,
                                    const Fw::LogSeverity& severity,
                                    Fw::TextLogString& text) {
    // EVS event IDs are 16 bits; the F Prime event ID is truncated to its low 16
    // bits. EVS truncates event text to CFE_MISSION_EVS_MAX_MESSAGE_LENGTH itself.
    // CFE_EVS_SendEventWithAppID is used with the app ID captured in init()
    // because this handler may run on a task unknown to CFE_ES
    CFE_Status_t status = CFE_EVS_SendEventWithAppID(static_cast<uint16>(id), EvsMirror::mapSeverity(severity),
                                                     this->m_appId, "%s", text.toChar());
    if (status != CFE_SUCCESS) {
        Fw::Logger::log("[ERROR] Failed to mirror event 0x%08x to EVS: 0x%08x\n", static_cast<U32>(id),
                        static_cast<U32>(status));
    }
}

// ----------------------------------------------------------------------
// Helper functions
// ----------------------------------------------------------------------

U16 EvsMirror ::mapSeverity(const Fw::LogSeverity& severity) {
    switch (severity.e) {
        case Fw::LogSeverity::FATAL:
            return CFE_EVS_EventType_CRITICAL;
        case Fw::LogSeverity::WARNING_HI:
        case Fw::LogSeverity::WARNING_LO:
            return CFE_EVS_EventType_ERROR;
        case Fw::LogSeverity::DIAGNOSTIC:
            return CFE_EVS_EventType_DEBUG;
        case Fw::LogSeverity::COMMAND:
        case Fw::LogSeverity::ACTIVITY_HI:
        case Fw::LogSeverity::ACTIVITY_LO:
        default:
            return CFE_EVS_EventType_INFORMATION;
    }
}

}  // namespace FPrimeCfs
