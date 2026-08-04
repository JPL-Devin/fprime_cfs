// ======================================================================
// \title  EvsMirror.cpp
// \brief  cpp file for EvsMirror component implementation class
// ======================================================================

#include "FPrimeCfs/EvsMirror/EvsMirror.hpp"
#include "Fw/Logger/Logger.hpp"

// Some cFE versions (e.g. draco) use compound literals in inline functions,
// which is a GCC extension when compiled as C++
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
extern "C" {
#include "cfe.h"  // for CFE_EVS_SendEvent and event types
}
#pragma GCC diagnostic pop

static_assert(FW_ENABLE_TEXT_LOGGING, "EvsMirror requires text logging (FW_ENABLE_TEXT_LOGGING) to receive event text");

namespace FPrimeCfs {

// ----------------------------------------------------------------------
// Component construction and destruction
// ----------------------------------------------------------------------

EvsMirror ::EvsMirror(const char* const compName) : EvsMirrorComponentBase(compName) {}

EvsMirror ::~EvsMirror() {}

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
    CFE_Status_t status =
        CFE_EVS_SendEvent(static_cast<uint16>(id), EvsMirror::mapSeverity(severity), "%s", text.toChar());
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
