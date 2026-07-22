// ======================================================================
// \title  SchAppDriver.hpp
// \author mstarch
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

    //! Handler implementation for dataIn
    //!
    //! Port receiving routed cFS scheduler messages. Each received message
    //! triggers one tick on CycleOut.
    void dataIn_handler(FwIndexType portNum,  //!< The port number
                        Fw::Buffer& data,
                        const ComCfg::FrameContext& context) override;
};

}  // namespace FPrimeCfs

#endif
