// ======================================================================
// \title  EvsMirror.hpp
// \brief  hpp file for EvsMirror component implementation class
// ======================================================================

#ifndef FPrimeCfs_EvsMirror_HPP
#define FPrimeCfs_EvsMirror_HPP

#include "FPrimeCfs/EvsMirror/EvsMirrorComponentAc.hpp"

namespace FPrimeCfs {

class EvsMirror final : public EvsMirrorComponentBase {
  public:
    // ----------------------------------------------------------------------
    // Component construction and destruction
    // ----------------------------------------------------------------------

    //! Construct EvsMirror object
    explicit EvsMirror(const char* const compName  //!< The component name
    );

    //! Destroy EvsMirror object
    ~EvsMirror();

  private:
    // ----------------------------------------------------------------------
    // Handler implementations for typed input ports
    // ----------------------------------------------------------------------

    //! Handler implementation for TextLogger
    //!
    //! Sends the formatted event text to cFS EVS via CFE_EVS_SendEvent.
    void TextLogger_handler(FwIndexType portNum,              //!< The port number
                            FwEventIdType id,                 //!< Log ID
                            Fw::Time& timeTag,                //!< Time tag
                            const Fw::LogSeverity& severity,  //!< The severity argument
                            Fw::TextLogString& text           //!< Text of log message
                            ) override;

    // ----------------------------------------------------------------------
    // Helper functions
    // ----------------------------------------------------------------------

    //! Map an F Prime event severity to a cFS EVS event type
    static U16 mapSeverity(const Fw::LogSeverity& severity);
};

}  // namespace FPrimeCfs

#endif
