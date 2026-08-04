#ifndef COMCFSSUBTOPOLOGY_DEFS_HPP
#define COMCFSSUBTOPOLOGY_DEFS_HPP

#include <Fw/Logger/Logger.hpp>
#include <Fw/Types/MallocAllocator.hpp>
#include <Svc/BufferManager/BufferManager.hpp>
#include <cstring>
#include "ComCfsConfig/ComCfsSubtopologyConfig.hpp"
#include "FPrimeCfs/Subtopologies/ComCfs/ComCfsConfig/FppConstantsAc.hpp"

namespace ComCfs {
struct SubtopologyState {
    // Empty - no external state needed for ComCfs subtopology
};

struct TopologyState {
    SubtopologyState comCfs;
};
}  // namespace ComCfs

#endif
