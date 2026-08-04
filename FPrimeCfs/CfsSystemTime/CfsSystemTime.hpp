// ======================================================================
// \title  CfsSystemTime.hpp
// \brief  hpp file for CfsSystemTime component implementation class
// ======================================================================

#ifndef FPrimeCfs_CfsSystemTime_HPP
#define FPrimeCfs_CfsSystemTime_HPP

#include "FPrimeCfs/CfsSystemTime/CfsSystemTimeComponentAc.hpp"

namespace FPrimeCfs {

class CfsSystemTime final : public CfsSystemTimeComponentBase {
  public:
    // ----------------------------------------------------------------------
    // Component construction and destruction
    // ----------------------------------------------------------------------

    //! Construct CfsSystemTime object
    CfsSystemTime(const char* const compName  //!< The component name
    );

    //! Destroy CfsSystemTime object
    ~CfsSystemTime();

  private:
    // ----------------------------------------------------------------------
    // Handler implementations for user-defined typed input ports
    // ----------------------------------------------------------------------

    //! Handler implementation for timeGetPort
    //!
    //! Port to retrieve time
    void timeGetPort_handler(FwIndexType portNum,  //!< The port number
                             Fw::Time& time        //!< Reference to Time object
                             ) override;

    //! Handler implementation for cfsTimeConvert
    //!
    //! Port converting an F Prime time back to a cFS system time
    CfsTime cfsTimeConvert_handler(FwIndexType portNum,  //!< The port number
                                   const Fw::Time& time  //!< The F Prime time to convert
                                   ) override;
};

}  // namespace FPrimeCfs

#endif
