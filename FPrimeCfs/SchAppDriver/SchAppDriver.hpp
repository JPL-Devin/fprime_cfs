// ======================================================================
// \title  SchAppDriver.hpp
// \brief  hpp file for SchAppDriver component implementation class
// ======================================================================

#ifndef FPrimeCfs_SchAppDriver_HPP
#define FPrimeCfs_SchAppDriver_HPP

#include "FPrimeCfs/SchAppDriver/SchAppDriverComponentAc.hpp"

namespace FPrimeCfs {

class SchAppDriver final : public SchAppDriverComponentBase {
  public:
    // ----------------------------------------------------------------------
    // Component construction and destruction
    // ----------------------------------------------------------------------

    //! Construct SchAppDriver object
    SchAppDriver(const char* const compName  //!< The component name
    );

    //! Destroy SchAppDriver object
    ~SchAppDriver();

  private:
    // ----------------------------------------------------------------------
    // Handler implementations for typed input ports
    // ----------------------------------------------------------------------

    //! Handler implementation for cfsCommandIn
    //!
    //! Port receiving routed cFS scheduler command messages. Emits one tick
    //! on CycleOut and returns the buffer via bufferReturnOut.
    void cfsCommandIn_handler(FwIndexType portNum,  //!< The port number
                              U8 functionCode,      //!< Function code from the cFS command secondary header
                              Fw::Buffer& data      //!< Command payload
                              ) override;
};

}  // namespace FPrimeCfs

#endif
