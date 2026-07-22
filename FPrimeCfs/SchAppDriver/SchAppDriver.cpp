// ======================================================================
// \title  SchAppDriver.cpp
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

void SchAppDriver ::cfsCommandIn_handler(FwIndexType portNum, U8 functionCode, Fw::Buffer& data) {
    Os::RawTime timestamp;
    timestamp.now();
    this->CycleOut_out(0, timestamp);
    this->bufferReturnOut_out(0, data);
}

}  // namespace FPrimeCfs
