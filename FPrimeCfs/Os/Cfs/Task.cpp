// ======================================================================
// \title FPrimeCfs/Os/Cfs/Task.cpp
// \brief implementation of cFE ES implementation of Os::Task
// ======================================================================
#include <atomic>

#include "FPrimeCfs/Os/Cfs/Task.hpp"
#include "Fw/Logger/Logger.hpp"
#include "Fw/Types/Assert.hpp"
#include "Fw/Types/StringUtils.hpp"

namespace Os {
namespace Cfs {
namespace Task {

// Out-of-class definitions for odr-used constexpr members (required in C++14)
constexpr CFE_ES_TaskPriority_Atom_t CfsTask::DEFAULT_PRIORITY;
constexpr FwSizeType CfsTask::MIN_PRIORITY;
constexpr FwSizeType CfsTask::MAX_PRIORITY;
constexpr FwSizeType CfsTask::DEFAULT_STACK_SIZE;
constexpr U32 CfsTask::HANDOFF_POLL_LIMIT;
constexpr U32 CfsTask::JOIN_POLL_DELAY_MS;

//! Handoff slot used to deliver the F Prime routine and argument to a newly
//! created child task: the cFE child task entry point takes no arguments, so
//! the routine/argument pair is published here by the parent before task
//! creation and consumed by the child on startup. s_handoff_taken serializes
//! the exchange: the parent busy-waits (with delays) on it before starting
//! another task, so at most one handoff is pending at a time. Task creation
//! is restricted to the app's main task by cFE, which also serializes the
//! writers.
static Os::Task::taskRoutine s_handoff_routine = nullptr;
static void* s_handoff_argument = nullptr;
static std::atomic<bool> s_handoff_taken(true);

//! \brief map a CFE_ES_CreateChildTask status to an Os::Task status
static Os::Task::Status map_create_status(CFE_Status_t cfe_status) {
    Os::Task::Status status = Os::Task::Status::UNKNOWN_ERROR;
    switch (cfe_status) {
        case CFE_SUCCESS:
            status = Os::Task::Status::OP_OK;
            break;
        case CFE_ES_BAD_ARGUMENT:
            status = Os::Task::Status::INVALID_PARAMS;
            break;
        // cFE reports both "not the app main task" and "out of task records"
        // through CFE_ES_ERR_CHILD_TASK_CREATE
        case CFE_ES_ERR_CHILD_TASK_CREATE:
            status = Os::Task::Status::ERROR_RESOURCES;
            break;
        // Returned when the caller is not a task belonging to any cFS app
        case CFE_ES_ERR_RESOURCEID_NOT_VALID:
            status = Os::Task::Status::INVALID_STATE;
            break;
        default:
            status = Os::Task::Status::UNKNOWN_ERROR;
            break;
    }
    return status;
}

void CfsTask::childTaskEntry() {
    // Consume the pending routine/argument pair published by start()
    Os::Task::taskRoutine routine = s_handoff_routine;
    void* argument = s_handoff_argument;
    s_handoff_taken.store(true);
    FW_ASSERT(routine != nullptr);
    routine(argument);
    // The F Prime routine returned: remove this child task's ES record
    CFE_ES_ExitChildTask();
}

void CfsTask::onStart() {}

Os::Task::Status CfsTask::start(const Arguments& arguments) {
    FW_ASSERT(arguments.m_routine != nullptr);

    // Affinity is not supported by the cFE child task API
    if (arguments.m_cpuAffinity != Os::Task::TASK_DEFAULT) {
        Fw::Logger::log("[WARNING] %s cpu affinity is not supported by cFE child tasks\n",
                        arguments.m_name.toChar());
    }

    // Priorities pass through directly as OSAL/cFS priorities (lower value =
    // more urgent), clamped to the valid OSAL range
    FwSizeType priority = static_cast<FwSizeType>(arguments.m_priority);
    if (arguments.m_priority == Os::Task::TASK_PRIORITY_DEFAULT) {
        priority = static_cast<FwSizeType>(CfsTask::DEFAULT_PRIORITY);
    } else if (priority < CfsTask::MIN_PRIORITY) {
        Fw::Logger::log("[WARNING] %s task priority of %" PRI_FwSizeType " clamped to %" PRI_FwSizeType "\n",
                        arguments.m_name.toChar(), priority, CfsTask::MIN_PRIORITY);
        priority = CfsTask::MIN_PRIORITY;
    } else if (priority > CfsTask::MAX_PRIORITY) {
        Fw::Logger::log("[WARNING] %s task priority of %" PRI_FwSizeType " clamped to %" PRI_FwSizeType "\n",
                        arguments.m_name.toChar(), priority, CfsTask::MAX_PRIORITY);
        priority = CfsTask::MAX_PRIORITY;
    }

    FwSizeType stackSize =
        (arguments.m_stackSize == Os::Task::TASK_DEFAULT) ? CfsTask::DEFAULT_STACK_SIZE : arguments.m_stackSize;

    // Publish the routine/argument pair for the child task entry point. The
    // preceding start() call's handoff must have completed (s_handoff_taken
    // is initialized true and set true by each child on startup).
    FW_ASSERT(s_handoff_taken.load());
    s_handoff_routine = arguments.m_routine;
    s_handoff_argument = arguments.m_routine_argument;
    s_handoff_taken.store(false);

    // OSAL limits task names to OS_MAX_API_NAME including the terminator;
    // keep the distinctive trailing characters when truncating
    char name[OS_MAX_API_NAME];
    const char* fullName = arguments.m_name.toChar();
    const FwSizeType nameLength = arguments.m_name.length();
    if (nameLength >= static_cast<FwSizeType>(OS_MAX_API_NAME)) {
        fullName = fullName + (nameLength - (OS_MAX_API_NAME - 1));
        Fw::Logger::log("[WARNING] %s task name truncated to %s\n", arguments.m_name.toChar(), fullName);
    }
    (void)Fw::StringUtils::string_copy(name, fullName, static_cast<FwSizeType>(sizeof(name)));

    CFE_ES_TaskId_t taskId = CFE_ES_TASKID_UNDEFINED;
    CFE_Status_t cfe_status = CFE_ES_CreateChildTask(
        &taskId, name, CfsTask::childTaskEntry, CFE_ES_TASK_STACK_ALLOCATE,
        static_cast<size_t>(stackSize), static_cast<CFE_ES_TaskPriority_Atom_t>(priority), 0);
    if (cfe_status != CFE_SUCCESS) {
        // The child task will never run: reclaim the handoff slot
        s_handoff_taken.store(true);
        Fw::Logger::log("[ERROR] %s failed to create cFE child task: 0x%08" PRIx32 "\n", arguments.m_name.toChar(),
                        static_cast<uint32>(cfe_status));
        return map_create_status(cfe_status);
    }

    // Wait (bounded) for the child task to consume its routine/argument pair
    // before allowing another handoff to be published
    U32 polls = 0;
    for (polls = 0; polls < CfsTask::HANDOFF_POLL_LIMIT; polls++) {
        if (s_handoff_taken.load()) {
            break;
        }
        (void)OS_TaskDelay(1);
    }
    // A created child task that never runs indicates a system-level fault
    FW_ASSERT(polls < CfsTask::HANDOFF_POLL_LIMIT, static_cast<FwAssertArgType>(polls));

    this->m_handle.m_task_id = taskId;
    this->m_handle.m_is_valid = true;
    return Os::Task::Status::OP_OK;
}

Os::Task::Status CfsTask::join() {
    if (not this->m_handle.m_is_valid) {
        return Os::Task::Status::INVALID_HANDLE;
    }
    // cFE has no native join: poll until the child task record disappears.
    // This loop is unbounded by design, matching join semantics (blocks until
    // the task exits), as with pthread_join in the Posix implementation.
    for (;;) {
        CFE_ES_TaskInfo_t info;
        CFE_Status_t status = CFE_ES_GetTaskInfo(&info, this->m_handle.m_task_id);
        if (status != CFE_SUCCESS) {
            break;
        }
        (void)OS_TaskDelay(CfsTask::JOIN_POLL_DELAY_MS);
    }
    this->m_handle.m_is_valid = false;
    return Os::Task::Status::OP_OK;
}

TaskHandle* CfsTask::getHandle() {
    return &this->m_handle;
}

// Note: not implemented for cFE child tasks. cFE ES provides no child task
// suspend/resume API; this matches the Posix implementation's behavior.
void CfsTask::suspend(Os::Task::SuspensionType suspensionType) {
    FW_ASSERT(false);
}

void CfsTask::resume() {
    FW_ASSERT(false);
}

Os::Task::Status CfsTask::_delay(const Fw::TimeInterval& interval) {
    // Round up to whole milliseconds: OS_TaskDelay has millisecond granularity
    U32 delay_ms = (interval.getSeconds() * 1000) + ((interval.getUSeconds() + 999) / 1000);
    int32 status = OS_TaskDelay(delay_ms);
    return (status == OS_SUCCESS) ? Os::Task::Status::OP_OK : Os::Task::Status::DELAY_ERROR;
}

}  // end namespace Task
}  // end namespace Cfs
}  // end namespace Os
