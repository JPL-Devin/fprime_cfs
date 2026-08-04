/**
 * @file
 *   Minimal cFE time services stub header for CfsSystemTime unit testing.
 */
#ifndef FPRIME_CFS_UT_STUB_CFE_TIME_H
#define FPRIME_CFS_UT_STUB_CFE_TIME_H

#include "cfe.h"

#if defined(__cplusplus)
extern "C" {
#endif

/** cFS system time: seconds and 2^-32 second subseconds */
typedef struct {
    uint32 Seconds;
    uint32 Subseconds;
} CFE_TIME_SysTime_t;

/** Get the current spacecraft time */
CFE_TIME_SysTime_t CFE_TIME_GetTime(void);

/** Convert 2^-32 second subseconds to microseconds */
uint32 CFE_TIME_Sub2MicroSecs(uint32 SubSeconds);

/** Convert microseconds to 2^-32 second subseconds */
uint32 CFE_TIME_Micro2SubSecs(uint32 MicroSeconds);

#if defined(__cplusplus)
}
#endif

#endif
