// ======================================================================
// \title  SchAppDriver.cpp
// \author mstarch
// \brief  cpp file for SchAppDriver component implementation class
// ======================================================================

#include "FPrimeCfs/SchAppDriver/SchAppDriver.hpp"
#include "Os/RawTime.hpp"

namespace FPrimeCfs {

// ----------------------------------------------------------------------
// Component construction and destruction
// ----------------------------------------------------------------------

SchAppDriver ::SchAppDriver(const char* const compName) : SchAppDriverComponentBase(compName) {}

SchAppDriver ::~SchAppDriver() {}

// ----------------------------------------------------------------------
// Handler implementations for typed input ports
// ----------------------------------------------------------------------

void SchAppDriver ::dataIn_handler(FwIndexType portNum, Fw::Buffer& data, const ComCfg::FrameContext& context) {
    Os::RawTime timestamp;
    timestamp.now();
    this->CycleOut_out(0, timestamp);
    this->dataReturnOut_out(0, data, context);
}

}  // namespace FPrimeCfs
