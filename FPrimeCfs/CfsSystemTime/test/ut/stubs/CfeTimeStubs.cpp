// ======================================================================
// \title  CfeTimeStubs.cpp
// \brief  Implementation of the cFE time stub layer for CfsSystemTime unit testing
// ======================================================================
#include "CfeTimeStubs.hpp"

#include <cstring>

namespace CfeStub {

static TimeState s_timeState;

TimeState& timeState() {
    return s_timeState;
}

void resetTime() {
    (void)std::memset(&s_timeState, 0, sizeof(s_timeState));
}

}  // namespace CfeStub

extern "C" {

CFE_TIME_SysTime_t CFE_TIME_GetTime(void) {
    CfeStub::s_timeState.getTimeCount++;
    return CfeStub::s_timeState.currentTime;
}

uint32 CFE_TIME_Sub2MicroSecs(uint32 SubSeconds) {
    return static_cast<uint32>((static_cast<uint64_t>(SubSeconds) * 1000000u) >> 32);
}

uint32 CFE_TIME_Micro2SubSecs(uint32 MicroSeconds) {
    if (MicroSeconds >= 1000000u) {
        return 0xFFFFFFFFu;
    }
    return static_cast<uint32>((static_cast<uint64_t>(MicroSeconds) << 32) / 1000000u);
}

}  // extern "C"
