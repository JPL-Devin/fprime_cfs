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
    EvsMirror(const char* const compName  //!< The component name
    );

    //! Destroy EvsMirror object
    ~EvsMirror();

  private:
    // ----------------------------------------------------------------------
    // Handler implementations for typed input ports
    // ----------------------------------------------------------------------

    //! Handler implementation for logIn
    //!
    //! Forwards the event unchanged on logOut. When text logging is disabled,
    //! also mirrors the event to cFS EVS in a compact identifier form.
    void logIn_handler(FwIndexType portNum,              //!< The port number
                       FwEventIdType id,                 //!< Log ID
                       Fw::Time& timeTag,                //!< Time tag
                       const Fw::LogSeverity& severity,  //!< The severity argument
                       Fw::LogBuffer& args               //!< Buffer containing serialized log entry
                       ) override;

    //! Handler implementation for textLogIn
    //!
    //! Forwards the text event unchanged on textLogOut. When text logging is
    //! enabled, also mirrors the formatted event text to cFS EVS.
    void textLogIn_handler(FwIndexType portNum,              //!< The port number
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

    //! Send an event to cFS EVS, logging a console error on failure
    static void sendToEvs(FwEventIdType id, const Fw::LogSeverity& severity, const char* text);
};

}  // namespace FPrimeCfs

#endif
