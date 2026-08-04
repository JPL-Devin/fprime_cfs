#ifndef CFSCORESUBTOPOLOGY_DEFS_HPP
#define CFSCORESUBTOPOLOGY_DEFS_HPP

#include "FPrimeCfs/Subtopologies/CfsCore/CfsCoreConfig/FppConstantsAc.hpp"

namespace CfsCore {
// State for topology construction
struct SubtopologyState {
    // Empty - no external state needed for CfsCore subtopology
};

struct TopologyState {
    SubtopologyState cfsCore;
};
}  // namespace CfsCore

#endif
