/**
 * @file
 *   Minimal cFE stub header for EvsMirror unit testing.
 *
 *   Provides only the types, constants, and function prototypes used by the
 *   EvsMirror component. The functions are implemented by CfeStubs.cpp which
 *   records calls and allows tests to inject return values.
 */
#ifndef FPRIME_CFS_EVSMIRROR_UT_STUB_CFE_H
#define FPRIME_CFS_EVSMIRROR_UT_STUB_CFE_H

#include <stdint.h>

#if defined(__cplusplus)
extern "C" {
#endif

typedef uint16_t uint16;
typedef uint32_t uint32;
typedef int32_t int32;

typedef int32 CFE_Status_t;
typedef uint32 CFE_ES_AppId_t;

#define CFE_SUCCESS ((CFE_Status_t)0)
#define CFE_EVS_APP_NOT_REGISTERED ((CFE_Status_t)0xc2000005)
#define CFE_ES_APPID_UNDEFINED ((CFE_ES_AppId_t)0)

/* Event types (values match CFE_EVS_EventType_Enum_t) */
#define CFE_EVS_EventType_DEBUG (1)
#define CFE_EVS_EventType_INFORMATION (2)
#define CFE_EVS_EventType_ERROR (3)
#define CFE_EVS_EventType_CRITICAL (4)

/* Event services API (implemented by CfeStubs.cpp) */
CFE_Status_t CFE_EVS_SendEvent(uint16 EventID, uint16 EventType, const char* Spec, ...);
CFE_Status_t CFE_EVS_SendEventWithAppID(uint16 EventID, uint16 EventType, CFE_ES_AppId_t AppID, const char* Spec, ...);

/* Executive services API (implemented by CfeStubs.cpp) */
CFE_Status_t CFE_ES_GetAppID(CFE_ES_AppId_t* AppIdPtr);

#if defined(__cplusplus)
}
#endif

#endif
