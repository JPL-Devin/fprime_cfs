// ======================================================================
// \title  EvsMirror.hpp
// \brief  hpp file for EvsMirror component implementation class
// ======================================================================

#ifndef FPrimeCfs_EvsMirror_HPP
#define FPrimeCfs_EvsMirror_HPP

#include "FPrimeCfs/EvsMirror/EvsMirrorComponentAc.hpp"

// Some cFE versions (e.g. draco) use compound literals in inline functions,
// which is a GCC extension when compiled as C++
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
extern "C" {
#include "cfe.h"  // for CFE_ES_AppId_t
}
#pragma GCC diagnostic pop

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

    //! Initialize EvsMirror object
    //!
    //! Overrides the base initialization to also capture the host cFS
    //! application ID. EVS resolves the calling task's application context,
    //! but the TextLogger handler runs on F Prime threads that are not
    //! created through CFE_ES and therefore have no ES task record, making
    //! CFE_EVS_SendEvent fail with CFE_EVS_APP_ILLEGAL_APP_ID. init() runs
    //! on the app's main (ES-registered) task during topology setup, so the
    //! app ID is captured here and events are mirrored with
    //! CFE_EVS_SendEventWithAppID instead.
    void init(FwEnumStoreType instance = 0  //!< The instance number
    );

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

    // ----------------------------------------------------------------------
    // Member variables
    // ----------------------------------------------------------------------

    //! Host cFS application ID, captured on the main task in init()
    CFE_ES_AppId_t m_appId;
};

}  // namespace FPrimeCfs

#endif
