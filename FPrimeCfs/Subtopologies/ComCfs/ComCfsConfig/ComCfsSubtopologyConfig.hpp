#ifndef COMCFSSUBTOPOLOGY_CONFIG_HPP
#define COMCFSSUBTOPOLOGY_CONFIG_HPP

#include "Fw/Types/MallocAllocator.hpp"
#include "config/ApidEnumAc.hpp"

namespace ComCfs {
namespace Allocation {
extern Fw::MemAllocator& memAllocator;
}
namespace BridgeConfig {
//! The APID used for cFS commands sent through the command app bridge
//! (ComCfs::cmdBridge). Projects set this to the command message ID APID of
//! the destination cFS application.
extern const ComCfg::Apid::T commandApid;
}  // namespace BridgeConfig
}  // namespace ComCfs

#endif
