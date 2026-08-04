// ======================================================================
// \title  CfeTimeStubs.hpp
// \brief  Test control interface for the cFE time stub layer
//
// Tests use this interface to inject the current time returned by
// CFE_TIME_GetTime.
// ======================================================================
#ifndef FPRIME_CFS_UT_CFE_TIME_STUBS_HPP
#define FPRIME_CFS_UT_CFE_TIME_STUBS_HPP

#include "cfe.h"

namespace CfeStub {

//! Shared state of the cFE time stub layer
struct TimeState {
    CFE_TIME_SysTime_t currentTime;  //!< Time returned by CFE_TIME_GetTime
    unsigned int getTimeCount;       //!< Number of CFE_TIME_GetTime calls
};

//! Get the stub state singleton
TimeState& timeState();

//! Reset the stub state to defaults (zero time, no records)
void resetTime();

}  // namespace CfeStub

#endif
