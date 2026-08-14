// ======================================================================
// \title FPrimeCfs/Os/Cfs/test/ut/stubs/CfeEsStubs.cpp
// \brief recording cFE ES/OSAL stubs used by CfsTask unit tests
// ======================================================================
#include "CfeEsStubs.hpp"

#include <chrono>
#include <cstring>
#include <thread>

namespace CfeEsStub {

State& state() {
    static State s_state;
    return s_state;
}

void reset() {
    State& s = state();
    (void)std::memset(&s, 0, sizeof(State));
    s.createStatus = CFE_SUCCESS;
    s.taskDelayStatus = OS_SUCCESS;
    s.createdTaskId = 0x42001;
    s.childRunMode = RUN_CHILD_INLINE;
}

}  // namespace CfeEsStub

extern "C" CFE_Status_t CFE_ES_CreateChildTask(CFE_ES_TaskId_t* TaskIdPtr,
                                               const char* TaskName,
                                               CFE_ES_ChildTaskMainFuncPtr_t FunctionPtr,
                                               CFE_ES_StackPointer_t StackPtr,
                                               size_t StackSize,
                                               CFE_ES_TaskPriority_Atom_t Priority,
                                               uint32 Flags) {
    CfeEsStub::State& s = CfeEsStub::state();
    s.createCount++;
    CfeEsStub::CreateChildTaskCall& call = s.lastCreate;
    call.stackSize = StackSize;
    call.priority = Priority;
    call.flags = Flags;
    call.stackPtr = StackPtr;
    call.name[0] = '\0';
    if (TaskName != nullptr) {
        (void)std::strncpy(call.name, TaskName, sizeof call.name - 1);
        call.name[sizeof call.name - 1] = '\0';
    }
    if (s.createStatus != CFE_SUCCESS) {
        return s.createStatus;
    }
    if (TaskIdPtr != nullptr) {
        *TaskIdPtr = s.createdTaskId;
    }
    if (s.childRunMode == CfeEsStub::RUN_CHILD_INLINE) {
        // Simulate the child task running to completion before create returns
        FunctionPtr();
    } else {
        s.deferredEntry = FunctionPtr;
    }
    return CFE_SUCCESS;
}

extern "C" CFE_Status_t CFE_ES_GetTaskInfo(CFE_ES_TaskInfo_t* TaskInfo, CFE_ES_TaskId_t TaskId) {
    CfeEsStub::State& s = CfeEsStub::state();
    s.getTaskInfoCount++;
    if (s.taskAlivePolls > 0) {
        s.taskAlivePolls--;
        if (TaskInfo != nullptr) {
            TaskInfo->TaskId = TaskId;
        }
        return CFE_SUCCESS;
    }
    return CFE_ES_ERR_RESOURCEID_NOT_VALID;
}

extern "C" void CFE_ES_ExitChildTask(void) {
    CfeEsStub::state().exitChildTaskCount++;
}

extern "C" int32 OS_TaskDelay(uint32 millisecond) {
    CfeEsStub::State& s = CfeEsStub::state();
    s.taskDelayCount++;
    // Sleep briefly so polling loops in the code under test yield to other
    // threads (e.g. the emulated asynchronous child task)
    (void)millisecond;
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    return s.taskDelayStatus;
}
