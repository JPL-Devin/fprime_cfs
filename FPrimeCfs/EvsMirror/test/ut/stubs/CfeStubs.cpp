// ======================================================================
// \title  CfeStubs.cpp
// \brief  Implementation of the cFE EVS stub layer for EvsMirror unit tests
// ======================================================================
#include "CfeStubs.hpp"
#include <cstdarg>
#include <cstdio>
#include <cstring>

namespace CfeStub {

State& state() {
    static State s_state;
    return s_state;
}

void reset() {
    (void)memset(&state(), 0, sizeof(State));
    state().sendEventStatus = CFE_SUCCESS;
    state().getAppIdStatus = CFE_SUCCESS;
}

}  // namespace CfeStub

extern "C" CFE_Status_t CFE_EVS_SendEvent(uint16 EventID, uint16 EventType, const char* Spec, ...) {
    CfeStub::State& s = CfeStub::state();
    if (s.sendEventCount < CfeStub::STUB_MAX_ENTRIES) {
        CfeStub::SendEventCall& call = s.sendEventCalls[s.sendEventCount];
        call.eventId = EventID;
        call.eventType = EventType;
        call.appId = CFE_ES_APPID_UNDEFINED;
        va_list args;
        va_start(args, Spec);
        (void)vsnprintf(call.text, sizeof call.text, Spec, args);
        va_end(args);
    }
    // Counts every call; only the first STUB_MAX_ENTRIES calls are recorded in sendEventCalls
    s.sendEventCount++;
    return s.sendEventStatus;
}

extern "C" CFE_Status_t CFE_EVS_SendEventWithAppID(uint16 EventID, uint16 EventType, CFE_ES_AppId_t AppID, const char* Spec, ...) {
    CfeStub::State& s = CfeStub::state();
    if (s.sendEventCount < CfeStub::STUB_MAX_ENTRIES) {
        CfeStub::SendEventCall& call = s.sendEventCalls[s.sendEventCount];
        call.eventId = EventID;
        call.eventType = EventType;
        call.appId = AppID;
        va_list args;
        va_start(args, Spec);
        (void)vsnprintf(call.text, sizeof call.text, Spec, args);
        va_end(args);
    }
    // Counts every call; only the first STUB_MAX_ENTRIES calls are recorded in sendEventCalls
    s.sendEventCount++;
    return s.sendEventStatus;
}

extern "C" CFE_Status_t CFE_ES_GetAppID(CFE_ES_AppId_t* AppIdPtr) {
    CfeStub::State& s = CfeStub::state();
    s.getAppIdCount++;
    if (s.getAppIdStatus == CFE_SUCCESS) {
        *AppIdPtr = s.appId;
    }
    return s.getAppIdStatus;
}
