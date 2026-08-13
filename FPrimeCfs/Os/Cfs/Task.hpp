// ======================================================================
// \title FPrimeCfs/Os/Cfs/Task.hpp
// \brief definitions of cFE ES implementation of Os::Task
// ======================================================================
#ifndef FPrimeCfs_Os_Cfs_Task_hpp_
#define FPrimeCfs_Os_Cfs_Task_hpp_

#include <Fw/FPrimeBasicTypes.hpp>
#include <Os/Task.hpp>

// Some cFE versions (e.g. draco) use compound literals in inline functions,
// which is a GCC extension when compiled as C++
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
extern "C" {
#include "cfe.h"
}
#pragma GCC diagnostic pop

namespace Os {
namespace Cfs {
namespace Task {

//! TaskHandle class definition for cFE ES child task implementations.
//!
struct CfsTaskHandle : public TaskHandle {
    //! cFE ES child task identifier
    CFE_ES_TaskId_t m_task_id = CFE_ES_TASKID_UNDEFINED;
    //! Is the above identifier valid
    bool m_is_valid = false;
};

//! \brief cFE ES child task implementation of Os::Task
//!
//! Creates F Prime tasks as cFE ES child tasks (CFE_ES_CreateChildTask) of the
//! hosting cFS application rather than raw OS threads. This gives every
//! F Prime task an ES task record, so cFE APIs that resolve the calling
//! task's application context (e.g. CFE_ES_GetAppID, CFE_EVS_SendEvent) work
//! from F Prime task contexts.
//!
//! Constraint: cFE only permits child task creation from the application's
//! main task. Tasks must therefore be started during topology setup (which
//! runs on the app's main task); start() returns an error status otherwise.
class CfsTask : public TaskInterface {
  public:
    //! OSAL priority used when the F Prime task supplies TASK_PRIORITY_DEFAULT.
    //! 200 places default tasks well below (less urgent than) cFS core and
    //! typical application task priorities (OSAL: lower number = more urgent).
    static constexpr CFE_ES_TaskPriority_Atom_t DEFAULT_PRIORITY = 200;

    //! Lowest valid OSAL priority value (most urgent). 0 is reserved.
    static constexpr FwSizeType MIN_PRIORITY = 1;

    //! Highest valid OSAL priority value (least urgent), i.e. OS_MAX_TASK_PRIORITY
    static constexpr FwSizeType MAX_PRIORITY = 255;

    //! Stack size used when the F Prime task supplies TASK_DEFAULT.
    //! Matches the deployment's default F Prime task stack size (64KiB).
    static constexpr FwSizeType DEFAULT_STACK_SIZE = 64 * 1024;

    //! Maximum number of 1ms polls waiting for the child task to take its
    //! routine argument (bounds the handoff loop; 10 seconds total)
    static constexpr U32 HANDOFF_POLL_LIMIT = 10000;

    //! Delay between join() polls of the child task's existence, in milliseconds
    static constexpr U32 JOIN_POLL_DELAY_MS = 10;

    //! \brief default constructor
    CfsTask() = default;

    //! \brief default virtual destructor
    ~CfsTask() = default;

    //! \brief copy constructor is forbidden
    CfsTask(const CfsTask& other) = delete;

    //! \brief assignment operator is forbidden
    CfsTask& operator=(const CfsTask& other) = delete;

    //! \brief perform required task start actions
    void onStart() override;

    //! \brief start the task as a cFE ES child task
    //!
    //! Starts the task via CFE_ES_CreateChildTask. The routine argument is
    //! delivered through a serialized static handoff slot because the cFE
    //! child task entry point takes no arguments. Must be called from the
    //! hosting application's main task (a cFE restriction); an error status
    //! is returned otherwise.
    //!
    //! It is illegal for arguments.m_routine to be null.
    //!
    //! \param arguments: arguments supplied to the task start call
    //! \return status of the task start
    Status start(const Arguments& arguments) override;

    //! \brief block until the task has ended
    //!
    //! cFE has no native child task join; this polls CFE_ES_GetTaskInfo until
    //! the child task record no longer exists.
    //!
    //! \return status of the block
    Status join() override;

    //! \brief suspend the task (unsupported; asserts)
    //!
    //! cFE provides no child task suspend API; matches the Posix
    //! implementation's behavior of asserting on use.
    //!
    //! \param suspensionType intentionality of the suspension
    void suspend(SuspensionType suspensionType) override;

    //! \brief resume a suspended task (unsupported; asserts)
    void resume() override;

    //! \brief delay the current task via OS_TaskDelay
    //!
    //! \param interval: delay time
    //! \return status of the delay
    Status _delay(const Fw::TimeInterval& interval) override;

    //! \brief return the underlying task handle (implementation specific)
    //! \return internal task handle representation
    TaskHandle* getHandle() override;

  private:
    //! \brief cFE ES child task entry point
    //!
    //! Takes the pending routine argument from the handoff slot, runs the
    //! F Prime task routine, and exits the child task cleanly when the
    //! routine returns.
    static void childTaskEntry();

    CfsTaskHandle m_handle;  //!< cFS task tracking
};

}  // end namespace Task
}  // end namespace Cfs
}  // end namespace Os
#endif  // FPrimeCfs_Os_Cfs_Task_hpp_
