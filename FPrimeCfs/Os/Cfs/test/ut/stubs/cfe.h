/**
 * @file
 *   Minimal cFE/OSAL stub header for Os::Cfs::Task unit testing.
 *
 *   Provides only the types, constants, and function prototypes used by the
 *   CfsTask implementation. The functions are implemented by CfeEsStubs.cpp
 *   which records calls and allows tests to inject return values and child
 *   task execution behavior.
 */
#ifndef FPRIME_CFS_OS_CFS_UT_STUB_CFE_H
#define FPRIME_CFS_OS_CFS_UT_STUB_CFE_H

#include <stddef.h>
#include <stdint.h>

#if defined(__cplusplus)
extern "C" {
#endif

typedef uint16_t uint16;
typedef uint32_t uint32;
typedef int32_t int32;

typedef int32 CFE_Status_t;
typedef uint32 CFE_ES_TaskId_t;
typedef uint16 CFE_ES_TaskPriority_Atom_t;
typedef void* CFE_ES_StackPointer_t;
typedef void (*CFE_ES_ChildTaskMainFuncPtr_t)(void);

#define CFE_SUCCESS ((CFE_Status_t)0)
#define CFE_ES_BAD_ARGUMENT ((CFE_Status_t)0xc4000002)
#define CFE_ES_ERR_CHILD_TASK_CREATE ((CFE_Status_t)0xc4000009)
#define CFE_ES_ERR_RESOURCEID_NOT_VALID ((CFE_Status_t)0xc400000b)

#define CFE_ES_TASKID_UNDEFINED ((CFE_ES_TaskId_t)0)
#define CFE_ES_TASK_STACK_ALLOCATE NULL

#define OS_MAX_API_NAME 20

/* Subset of CFE_ES_TaskInfo_t sufficient for CfsTask usage */
typedef struct {
    CFE_ES_TaskId_t TaskId;
    char TaskName[OS_MAX_API_NAME];
} CFE_ES_TaskInfo_t;

/* Executive services API (implemented by CfeEsStubs.cpp) */
CFE_Status_t CFE_ES_CreateChildTask(CFE_ES_TaskId_t* TaskIdPtr,
                                    const char* TaskName,
                                    CFE_ES_ChildTaskMainFuncPtr_t FunctionPtr,
                                    CFE_ES_StackPointer_t StackPtr,
                                    size_t StackSize,
                                    CFE_ES_TaskPriority_Atom_t Priority,
                                    uint32 Flags);
CFE_Status_t CFE_ES_GetTaskInfo(CFE_ES_TaskInfo_t* TaskInfo, CFE_ES_TaskId_t TaskId);
void CFE_ES_ExitChildTask(void);

/* OSAL API (implemented by CfeEsStubs.cpp) */
#define OS_SUCCESS (0)
#define OS_ERROR (-1)
int32 OS_TaskDelay(uint32 millisecond);

#if defined(__cplusplus)
}
#endif

#endif
