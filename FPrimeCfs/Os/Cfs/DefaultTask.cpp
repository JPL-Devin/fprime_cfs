// ======================================================================
// \title FPrimeCfs/Os/Cfs/DefaultTask.cpp
// \brief sets default Os::Task to the cFE ES child task implementation via linker
// ======================================================================
#include "FPrimeCfs/Os/Cfs/Task.hpp"
#include "Os/Delegate.hpp"
#include "Os/Task.hpp"

namespace Os {

TaskInterface* TaskInterface::getDelegate(TaskHandleStorage& aligned_new_memory) {
    return Os::Delegate::makeDelegate<TaskInterface, Os::Cfs::Task::CfsTask>(aligned_new_memory);
}

}  // namespace Os
