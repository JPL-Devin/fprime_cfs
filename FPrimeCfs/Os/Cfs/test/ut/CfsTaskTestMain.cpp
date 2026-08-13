// ======================================================================
// \title FPrimeCfs/Os/Cfs/test/ut/CfsTaskTestMain.cpp
// \brief unit tests for the cFE ES child task implementation of Os::Task
// ======================================================================
#include <gtest/gtest.h>

#include <atomic>
#include <chrono>
#include <thread>

#include "CfeEsStubs.hpp"
#include "FPrimeCfs/Os/Cfs/Task.hpp"
#include "Fw/Types/Assert.hpp"
#include "Fw/Types/String.hpp"

using Os::Cfs::Task::CfsTask;
using Os::Cfs::Task::CfsTaskHandle;

namespace {

// Assert hook that records asserts instead of aborting, for testing
// FW_ASSERT-on-unsupported behavior (REQ-OSTASKCFS-008)
class RecordingAssertHook : public Fw::AssertHook {
  public:
    void reportAssert(FILE_NAME_ARG file,
                      FwSizeType lineNo,
                      FwSizeType numArgs,
                      FwAssertArgType arg1,
                      FwAssertArgType arg2,
                      FwAssertArgType arg3,
                      FwAssertArgType arg4,
                      FwAssertArgType arg5,
                      FwAssertArgType arg6) override {
        this->m_asserts++;
    }
    void doAssert() override {}
    unsigned int m_asserts = 0;
};

// Routine capture state
struct RoutineRecord {
    std::atomic<unsigned int> calls{0};
    std::atomic<void*> lastArgument{nullptr};
};

RoutineRecord& routineRecord() {
    static RoutineRecord s_record;
    return s_record;
}

void testRoutine(void* argument) {
    routineRecord().lastArgument.store(argument);
    routineRecord().calls++;
}

class CfsTaskTest : public ::testing::Test {
  protected:
    void SetUp() override {
        CfeEsStub::reset();
        routineRecord().calls = 0;
        routineRecord().lastArgument = nullptr;
    }
    CfsTask m_task;
    U32 m_argument_target = 0;
};

// REQ-OSTASKCFS-001/002/006: nominal start creates a cFE child task with the
// supplied name/stack/priority, runs the routine with its argument, and the
// child exits via CFE_ES_ExitChildTask when the routine returns
TEST_F(CfsTaskTest, StartNominal) {
    Fw::String name("TestTask");
    Os::Task::Arguments arguments(name, testRoutine, &this->m_argument_target, 100, 32 * 1024);
    ASSERT_EQ(this->m_task.start(arguments), Os::Task::Status::OP_OK);

    CfeEsStub::State& s = CfeEsStub::state();
    ASSERT_EQ(s.createCount, 1u);
    ASSERT_STREQ(s.lastCreate.name, "TestTask");
    ASSERT_EQ(s.lastCreate.stackSize, static_cast<size_t>(32 * 1024));
    ASSERT_EQ(s.lastCreate.priority, static_cast<CFE_ES_TaskPriority_Atom_t>(100));
    ASSERT_EQ(s.lastCreate.stackPtr, nullptr);  // CFE_ES_TASK_STACK_ALLOCATE

    // Routine ran exactly once with the supplied argument
    ASSERT_EQ(routineRecord().calls.load(), 1u);
    ASSERT_EQ(routineRecord().lastArgument.load(), &this->m_argument_target);

    // Child exited its ES record after the routine returned
    ASSERT_EQ(s.exitChildTaskCount, 1u);

    // Handle holds the created task id
    CfsTaskHandle* handle = static_cast<CfsTaskHandle*>(this->m_task.getHandle());
    ASSERT_TRUE(handle->m_is_valid);
    ASSERT_EQ(handle->m_task_id, s.createdTaskId);
}

// REQ-OSTASKCFS-003: default priority sentinel maps to the defined default
TEST_F(CfsTaskTest, StartDefaultPriority) {
    Fw::String name("DefaultPrio");
    Os::Task::Arguments arguments(name, testRoutine);
    ASSERT_EQ(this->m_task.start(arguments), Os::Task::Status::OP_OK);
    ASSERT_EQ(CfeEsStub::state().lastCreate.priority, CfsTask::DEFAULT_PRIORITY);
}

// REQ-OSTASKCFS-002: default stack sentinel maps to the defined default
TEST_F(CfsTaskTest, StartDefaultStack) {
    Fw::String name("DefaultStack");
    Os::Task::Arguments arguments(name, testRoutine);
    ASSERT_EQ(this->m_task.start(arguments), Os::Task::Status::OP_OK);
    ASSERT_EQ(CfeEsStub::state().lastCreate.stackSize, static_cast<size_t>(CfsTask::DEFAULT_STACK_SIZE));
}

// REQ-OSTASKCFS-002: names longer than OS_MAX_API_NAME are truncated to their
// distinctive trailing characters
TEST_F(CfsTaskTest, StartNameTruncation) {
    Fw::String name("FPrimeApp.rateGroup1");  // 20 chars: exceeds the 19-char limit
    Os::Task::Arguments arguments(name, testRoutine);
    ASSERT_EQ(this->m_task.start(arguments), Os::Task::Status::OP_OK);
    ASSERT_STREQ(CfeEsStub::state().lastCreate.name, "PrimeApp.rateGroup1");
}

// REQ-OSTASKCFS-003: priorities pass through directly; out-of-range values
// clamp (0 is reserved in OSAL; the high clamp is unreachable when
// FwTaskPriorityType is 8 bits, since its maximum value is the default
// sentinel)
TEST_F(CfsTaskTest, StartPriorityClamping) {
    {
        Fw::String name("PassThrough");
        Os::Task::Arguments arguments(name, testRoutine, nullptr, 43);
        ASSERT_EQ(this->m_task.start(arguments), Os::Task::Status::OP_OK);
        ASSERT_EQ(CfeEsStub::state().lastCreate.priority, static_cast<CFE_ES_TaskPriority_Atom_t>(43));
    }
    {
        CfsTask task;
        Fw::String name("ClampLow");
        Os::Task::Arguments arguments(name, testRoutine, nullptr, 0);
        ASSERT_EQ(task.start(arguments), Os::Task::Status::OP_OK);
        ASSERT_EQ(CfeEsStub::state().lastCreate.priority, static_cast<CFE_ES_TaskPriority_Atom_t>(1));
    }
}

// REQ-OSTASKCFS-004: the routine/argument pair is delivered through the
// handoff slot even when the child task starts asynchronously
TEST_F(CfsTaskTest, StartAsynchronousChild) {
    CfeEsStub::state().childRunMode = CfeEsStub::DEFER_CHILD;

    // Emulate the asynchronous child task: wait for the entry point recorded
    // by the create stub, then run it on a separate thread
    std::thread child([]() {
        while (CfeEsStub::state().deferredEntry == nullptr) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
        CfeEsStub::state().deferredEntry();
    });

    Fw::String name("AsyncChild");
    Os::Task::Arguments arguments(name, testRoutine, &this->m_argument_target);
    ASSERT_EQ(this->m_task.start(arguments), Os::Task::Status::OP_OK);
    child.join();

    ASSERT_EQ(routineRecord().calls.load(), 1u);
    ASSERT_EQ(routineRecord().lastArgument.load(), &this->m_argument_target);
    ASSERT_EQ(CfeEsStub::state().exitChildTaskCount, 1u);
}

// REQ-OSTASKCFS-005: cFE errors map to Os::Task statuses and leave the handle
// invalid; the handoff slot is reclaimed so a subsequent start succeeds
TEST_F(CfsTaskTest, StartCreateFailures) {
    struct Case {
        CFE_Status_t cfeStatus;
        Os::Task::Status expected;
    };
    const Case cases[3] = {
        {CFE_ES_BAD_ARGUMENT, Os::Task::Status::INVALID_PARAMS},
        {CFE_ES_ERR_CHILD_TASK_CREATE, Os::Task::Status::ERROR_RESOURCES},
        {CFE_ES_ERR_RESOURCEID_NOT_VALID, Os::Task::Status::INVALID_STATE},
    };
    for (const Case& testCase : cases) {
        CfsTask task;
        CfeEsStub::state().createStatus = testCase.cfeStatus;
        Fw::String name("FailTask");
        Os::Task::Arguments arguments(name, testRoutine);
        ASSERT_EQ(task.start(arguments), testCase.expected);
        CfsTaskHandle* handle = static_cast<CfsTaskHandle*>(task.getHandle());
        ASSERT_FALSE(handle->m_is_valid);
        ASSERT_EQ(routineRecord().calls.load(), 0u);
    }
    // The handoff slot was reclaimed on failure: a subsequent start succeeds
    CfeEsStub::state().createStatus = CFE_SUCCESS;
    Fw::String name("Recovery");
    Os::Task::Arguments arguments(name, testRoutine);
    ASSERT_EQ(this->m_task.start(arguments), Os::Task::Status::OP_OK);
    ASSERT_EQ(routineRecord().calls.load(), 1u);
}

// REQ-OSTASKCFS-007: join blocks (polling) until the child task record no
// longer exists
TEST_F(CfsTaskTest, JoinNominal) {
    Fw::String name("JoinTask");
    Os::Task::Arguments arguments(name, testRoutine);
    ASSERT_EQ(this->m_task.start(arguments), Os::Task::Status::OP_OK);

    CfeEsStub::State& s = CfeEsStub::state();
    s.taskAlivePolls = 3;
    unsigned int delaysBefore = s.taskDelayCount;
    ASSERT_EQ(this->m_task.join(), Os::Task::Status::OP_OK);
    // 3 alive polls + 1 final poll observing the task gone
    ASSERT_EQ(s.getTaskInfoCount, 4u);
    // One delay per alive poll
    ASSERT_EQ(s.taskDelayCount - delaysBefore, 3u);
    // A second join reports the invalid handle
    ASSERT_EQ(this->m_task.join(), Os::Task::Status::INVALID_HANDLE);
}

// REQ-OSTASKCFS-007: join on a never-started task reports INVALID_HANDLE
TEST_F(CfsTaskTest, JoinInvalidHandle) {
    ASSERT_EQ(this->m_task.join(), Os::Task::Status::INVALID_HANDLE);
}

// REQ-OSTASKCFS-008: suspend and resume are unsupported and assert
TEST_F(CfsTaskTest, SuspendResumeAssert) {
    RecordingAssertHook hook;
    hook.registerHook();
    this->m_task.suspend(Os::Task::SuspensionType::INTENTIONAL);
    ASSERT_EQ(hook.m_asserts, 1u);
    this->m_task.resume();
    ASSERT_EQ(hook.m_asserts, 2u);
    hook.deregisterHook();
}

// REQ-OSTASKCFS-009: delay delegates to OS_TaskDelay and maps failures
TEST_F(CfsTaskTest, Delay) {
    ASSERT_EQ(this->m_task._delay(Fw::TimeInterval(0, 500000)), Os::Task::Status::OP_OK);
    ASSERT_EQ(CfeEsStub::state().taskDelayCount, 1u);
    CfeEsStub::state().taskDelayStatus = OS_ERROR;
    ASSERT_EQ(this->m_task._delay(Fw::TimeInterval(1, 0)), Os::Task::Status::DELAY_ERROR);
}

}  // namespace

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
