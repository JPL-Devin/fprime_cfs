# Os::Task cFE ES Implementation (Os_Task_Cfs)

## 1. Introduction

`Os_Task_Cfs` is an F Prime `Os::Task` implementation that creates F Prime
tasks as **cFE ES child tasks** (`CFE_ES_CreateChildTask`) of the hosting cFS
application, rather than raw OS threads.

### Motivation

When F Prime runs as a cFS application, tasks created through the default
Posix `Os::Task` implementation are unknown to cFE Executive Services: ES has
no task record for them, so any cFE API that resolves the *calling task's*
application context fails from F Prime task contexts. Observed consequences:

- `CFE_EVS_SendEvent` fails with `CFE_EVS_APP_ILLEGAL_APP_ID` (0xc2000003).
- `CFE_EVS_SendEventWithAppID` is silently filtered, because
  `EVS_IsFiltered` -> `CFE_EVS_GetTypeEnable` re-resolves the calling task via
  `CFE_ES_GetAppID`.

Creating F Prime tasks as ES child tasks gives every F Prime task an ES task
record, so `CFE_ES_GetAppID` (and every API built on it) resolves correctly on
any F Prime task.

## 2. Requirements

| ID | Shall Statement | Verification |
|---|---|---|
| REQ-OSTASKCFS-001 | `start()` shall create the task via `CFE_ES_CreateChildTask`. | Unit test `StartNominal` |
| REQ-OSTASKCFS-002 | `start()` shall pass the task name (truncated to its trailing `OS_MAX_API_NAME - 1` characters when longer), stack size (default when `TASK_DEFAULT`), and priority to cFE and return `OP_OK` on success. | Unit tests `StartNominal`, `StartDefaultStack`, `StartNameTruncation` |
| REQ-OSTASKCFS-003 | `start()` shall pass F Prime priorities through directly as OSAL/cFS priorities (lower = more urgent), clamped to 1–255, with a defined default (200) for `TASK_PRIORITY_DEFAULT`. | Unit tests `StartDefaultPriority`, `StartPriorityClamping` |
| REQ-OSTASKCFS-004 | `start()` shall deliver the routine/argument pair to the child task through a serialized handoff with a handshake guaranteeing each child receives its own argument. | Unit test `StartAsynchronousChild` |
| REQ-OSTASKCFS-005 | On cFE error, `start()` shall return the corresponding `Os::TaskInterface::Status` and leave the handle invalid. | Unit test `StartCreateFailures` |
| REQ-OSTASKCFS-006 | The child task shall call `CFE_ES_ExitChildTask()` when the user routine returns. | Unit tests `StartNominal`, `StartAsynchronousChild` |
| REQ-OSTASKCFS-007 | `join()` shall block until the child task record no longer exists; `join()` on an invalid handle shall return `INVALID_HANDLE`. | Unit tests `JoinNominal`, `JoinInvalidHandle` |
| REQ-OSTASKCFS-008 | `suspend()`/`resume()` shall be unsupported and shall assert. | Unit test `SuspendResumeAssert` |
| REQ-OSTASKCFS-009 | `_delay()` shall delay via `OS_TaskDelay` (rounded up to whole milliseconds) and map failures to `DELAY_ERROR`. | Unit test `Delay` |
| REQ-OSTASKCFS-010 | `start()` shall be documented as callable only from an ES-registered main task (a cFE restriction); violations surface through the REQ-005 error path. | Inspection; unit test `StartCreateFailures` |

## 3. Design

### 3.1 Module layout

```
FPrimeCfs/Os/Cfs/
  Task.hpp / Task.cpp      Os::Cfs::Task::CfsTask (TaskInterface), CfsTaskHandle
  DefaultTask.cpp          TaskInterface::getDelegate -> CfsTask
  CMakeLists.txt           register_os_implementation("Task" Cfs Fw_Logger)
  test/ut/                 GTest suite + recording cFE ES/OSAL stubs
```

The implementation is selected per deployment:

```cmake
fprime_target_implementations("<deployment>" Os_Task_Cfs)
```

### 3.2 Routine/argument handoff

`CFE_ES_ChildTaskMainFuncPtr_t` takes no arguments, so the F Prime routine and
its argument are delivered through a static handoff slot:

```
start():                                  childTaskEntry():
  assert previous handoff complete          routine = s_handoff_routine
  publish routine + argument                argument = s_handoff_argument
  s_handoff_taken = false                   s_handoff_taken = true
  CFE_ES_CreateChildTask(entry, ...)        routine(argument)
  poll s_handoff_taken (bounded, 10 s)      CFE_ES_ExitChildTask()
```

The exchange is serialized: `start()` does not return until the child has
consumed its pair, so at most one handoff is pending. cFE restricts child
task creation to the app's main task, which also serializes the writers.

### 3.3 Priority and stack mapping

- Priorities pass through directly as OSAL priorities (lower = more urgent),
  clamped to [1, 255]; topology priorities must therefore be specified in
  cFS terms.
- `TASK_PRIORITY_DEFAULT` maps to OSAL 200 (well below cFS core priorities).
- `TASK_DEFAULT` stack maps to 64 KiB (the deployments' default F Prime task
  stack size); cFE allocates the stack (`CFE_ES_TASK_STACK_ALLOCATE`).
- CPU affinity is unsupported by the cFE API; a warning is logged if
  requested.

### 3.4 Constraints

- `start()` must be called from the app's **main task** — cFE rejects child
  task creation from child tasks. F Prime starts active-component tasks
  during topology setup, which runs on the app's main task.
- `suspend()`/`resume()` assert: cFE ES has no child task suspend API
  (matches the Posix implementation's behavior).
- `join()` polls `CFE_ES_GetTaskInfo` every 10 ms: cFE has no native join.

## 4. Unit Testing

The GTest suite (`test/ut/`) tests `CfsTask` directly against recording
cFE ES/OSAL stubs (`test/ut/stubs/`), which allow injecting create failures,
deferring child execution to a real thread (to exercise the asynchronous
handoff), and simulating a running-then-exited child for `join()`.
