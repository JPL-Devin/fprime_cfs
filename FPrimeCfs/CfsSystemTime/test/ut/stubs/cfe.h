/**
 * @file
 *   Minimal cFE stub header for CfsSystemTime unit testing.
 *
 *   Provides only the types and function prototypes used by the
 *   CfsSystemTime component. The functions are implemented by
 *   CfeTimeStubs.cpp which allows tests to inject the current time.
 */
#ifndef FPRIME_CFS_UT_STUB_CFE_H
#define FPRIME_CFS_UT_STUB_CFE_H

#include <stdint.h>

#if defined(__cplusplus)
extern "C" {
#endif

typedef uint32_t uint32;

#if defined(__cplusplus)
}
#endif

#include "cfe_time.h"

#endif
