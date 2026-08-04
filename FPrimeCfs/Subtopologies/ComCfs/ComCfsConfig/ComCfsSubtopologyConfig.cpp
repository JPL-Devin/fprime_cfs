#include "ComCfsSubtopologyConfig.hpp"

namespace ComCfs {
namespace Allocation {
// This instance can be changed to use a different allocator in the ComCfs Subtopology
Fw::MallocAllocator mallocatorInstance;
Fw::MemAllocator& memAllocator = mallocatorInstance;
}  // namespace Allocation
namespace BridgeConfig {
// Placeholder default; projects override this configuration module to supply
// the command message ID APID of the destination cFS application
const ComCfg::Apid::T commandApid = ComCfg::Apid::FW_PACKET_COMMAND;
}  // namespace BridgeConfig
}  // namespace ComCfs
