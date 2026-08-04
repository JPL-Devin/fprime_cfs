// ======================================================================
// \title  CfeStubs.hpp
// \brief  Test control interface for the cFE EVS stub layer
//
// Tests use this interface to inject return statuses and inspect the
// EVS events the component under test sent.
// ======================================================================
#ifndef FPRIME_CFS_EVSMIRROR_UT_CFE_STUBS_HPP
#define FPRIME_CFS_EVSMIRROR_UT_CFE_STUBS_HPP

#include "cfe.h"

namespace CfeStub {

//! Maximum recorded entries in the stub
static const unsigned int STUB_MAX_ENTRIES = 32;
//! Maximum formatted text bytes captured per sent event
static const unsigned int STUB_MAX_TEXT = 256;

//! Record of a CFE_EVS_SendEvent call
struct SendEventCall {
    uint16 eventId;
    uint16 eventType;
    char text[STUB_MAX_TEXT];
};

//! Shared state of the cFE stub layer
struct State {
    // Injectable return status
    CFE_Status_t sendEventStatus;

    // Call records
    unsigned int sendEventCount;
    SendEventCall sendEventCalls[STUB_MAX_ENTRIES];
};

//! Get the stub state singleton
State& state();

//! Reset the stub state to defaults (status CFE_SUCCESS, no records)
void reset();

}  // namespace CfeStub

#endif
