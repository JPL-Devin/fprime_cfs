// ======================================================================
// \title  CfsSystemTime.cpp
// \brief  cpp file for CfsSystemTime component implementation class
// ======================================================================

#include "FPrimeCfs/CfsSystemTime/CfsSystemTime.hpp"
#include "Fw/FPrimeBasicTypes.hpp"

// Some cFE versions (e.g. draco) use compound literals in inline functions,
// which is a GCC extension when compiled as C++
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
extern "C" {
    #include "cfe.h"
    #include "cfe_time.h"  // for CFE_TIME_GetTime and subsecond conversions
}
#pragma GCC diagnostic pop

namespace FPrimeCfs {

// ----------------------------------------------------------------------
// Component construction and destruction
// ----------------------------------------------------------------------

CfsSystemTime ::CfsSystemTime(const char* const compName) : CfsSystemTimeComponentBase(compName) {}

CfsSystemTime ::~CfsSystemTime() {}

// ----------------------------------------------------------------------
// Handler implementations for user-defined typed input ports
// ----------------------------------------------------------------------

void CfsSystemTime ::timeGetPort_handler(FwIndexType portNum, Fw::Time& time) {
    const CFE_TIME_SysTime_t cfsTime = CFE_TIME_GetTime();
    time.set(TimeBase::TB_WORKSTATION_TIME, static_cast<U32>(cfsTime.Seconds),
             static_cast<U32>(CFE_TIME_Sub2MicroSecs(cfsTime.Subseconds)));
}

CfsTime CfsSystemTime ::cfsTimeConvert_handler(FwIndexType portNum, const Fw::Time& time) {
    return CfsTime(time.getSeconds(), CFE_TIME_Micro2SubSecs(time.getUSeconds()));
}

}  // namespace FPrimeCfs
