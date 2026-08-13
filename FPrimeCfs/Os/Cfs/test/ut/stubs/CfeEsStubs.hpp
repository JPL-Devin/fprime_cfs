// ======================================================================
// \title FPrimeCfs/Os/Cfs/test/ut/stubs/CfeEsStubs.hpp
// \brief test state for the cFE ES/OSAL stubs used by CfsTask unit tests
// ======================================================================
#ifndef FPRIME_CFS_OS_CFS_UT_CFE_ES_STUBS_HPP
#define FPRIME_CFS_OS_CFS_UT_CFE_ES_STUBS_HPP

#include "cfe.h"

namespace CfeEsStub {

static const unsigned int STUB_MAX_NAME = 64;

//! Modes for how the CFE_ES_CreateChildTask stub runs the child entry point
enum ChildRunMode {
    RUN_CHILD_INLINE,   //!< run the child entry function before returning (default)
    DEFER_CHILD,        //!< record the entry function; the test runs it later
};

//! Record of a CFE_ES_CreateChildTask call
struct CreateChildTaskCall {
    char name[STUB_MAX_NAME];
    size_t stackSize;
    CFE_ES_TaskPriority_Atom_t priority;
    uint32 flags;
    CFE_ES_StackPointer_t stackPtr;
};

struct State {
    // Injectable return statuses
    CFE_Status_t createStatus;
    int32 taskDelayStatus;

    // Injectable task id written by CFE_ES_CreateChildTask
    CFE_ES_TaskId_t createdTaskId;

    // How the stub executes the child entry point
    ChildRunMode childRunMode;

    // Entry point recorded when childRunMode == DEFER_CHILD
    CFE_ES_ChildTaskMainFuncPtr_t deferredEntry;

    // Number of CFE_ES_GetTaskInfo calls that report the task alive before
    // reporting it gone (simulates a running-then-exited child for join())
    unsigned int taskAlivePolls;

    // Call counters
    unsigned int createCount;
    unsigned int getTaskInfoCount;
    unsigned int exitChildTaskCount;
    unsigned int taskDelayCount;

    // Last recorded create call
    CreateChildTaskCall lastCreate;
};

//! Access the stub state singleton
State& state();

//! Reset stub state to nominal defaults
void reset();

}  // namespace CfeEsStub

#endif
