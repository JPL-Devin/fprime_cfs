// ======================================================================
// \title  EvsMirror.cpp
// \brief  cpp file for EvsMirror component implementation class
// ======================================================================

#include "FPrimeCfs/EvsMirror/EvsMirror.hpp"
#include <cinttypes>
#include <cstdio>
#include "Fw/Logger/Logger.hpp"

// Some cFE versions (e.g. draco) use compound literals in inline functions,
// which is a GCC extension when compiled as C++
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
extern "C" {
#include "cfe.h"  // for CFE_EVS_SendEvent and event types
}
#pragma GCC diagnostic pop

namespace FPrimeCfs {

// ----------------------------------------------------------------------
// Component construction and destruction
// ----------------------------------------------------------------------

EvsMirror ::EvsMirror(const char* const compName) : EvsMirrorComponentBase(compName) {}

EvsMirror ::~EvsMirror() {}

// ----------------------------------------------------------------------
// Handler implementations for typed input ports
// ----------------------------------------------------------------------

void EvsMirror ::logIn_handler(FwIndexType portNum,
                               FwEventIdType id,
                               Fw::Time& timeTag,
                               const Fw::LogSeverity& severity,
                               Fw::LogBuffer& args) {
#if !FW_ENABLE_TEXT_LOGGING
    // Text logging is disabled: the formatted event text is unavailable, so mirror
    // a compact identifier form. Ground systems resolve the ID via the dictionary.
    char compact[64];
    (void)snprintf(compact, sizeof compact, "F Prime EVR 0x%08" PRIx32 " severity %" PRIu32, static_cast<U32>(id),
                   static_cast<U32>(severity.e));
    EvsMirror::sendToEvs(id, severity, compact);
#endif
    if (this->isConnected_logOut_OutputPort(0)) {
        this->logOut_out(0, id, timeTag, severity, args);
    }
}

void EvsMirror ::textLogIn_handler(FwIndexType portNum,
                                   FwEventIdType id,
                                   Fw::Time& timeTag,
                                   const Fw::LogSeverity& severity,
                                   Fw::TextLogString& text) {
#if FW_ENABLE_TEXT_LOGGING
    EvsMirror::sendToEvs(id, severity, text.toChar());
#endif
    if (this->isConnected_textLogOut_OutputPort(0)) {
        this->textLogOut_out(0, id, timeTag, severity, text);
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

void EvsMirror ::sendToEvs(FwEventIdType id, const Fw::LogSeverity& severity, const char* text) {
    // EVS event IDs are 16 bits; the F Prime event ID is truncated to its low 16
    // bits. EVS truncates event text to CFE_MISSION_EVS_MAX_MESSAGE_LENGTH itself.
    CFE_Status_t status = CFE_EVS_SendEvent(static_cast<uint16>(id), EvsMirror::mapSeverity(severity), "%s", text);
    if (status != CFE_SUCCESS) {
        Fw::Logger::log("[ERROR] Failed to mirror event 0x%08x to EVS: 0x%08x\n", static_cast<U32>(id),
                        static_cast<U32>(status));
    }
}

}  // namespace FPrimeCfs
