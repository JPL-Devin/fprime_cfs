/**
 * @file
 *   Specification for the CfsBridge command and telemetry
 *   (generic) message data types for handling F Prime data opaquely.
 *
 * @note
 *   These constructs should be included into the applications who intend
 *   to work with F Prime bridged data.
 */
#ifndef FPRIME_CFS_CFS_BRIDGE_MSGSTRUCT_H
#define FPRIME_CFS_CFS_BRIDGE_MSGSTRUCT_H

#if defined(__cplusplus)
extern "C" {
#endif

#include "cfe_msg_hdr.h"

// TODO: Use the F Prime Fw::Com::Buffer max size, or some other viable limit
#define FPRIME_RAW_FRAME_MAX_SIZE 2048

//! \brief a generic command for F Prime command and uplink data
//!
//! This structure is used to encapsulate F Prime command and uplink data in a format compatible with the cFS software
//! bus. It has the appropriate header and a data field large enough to hold all F Prime uplink data.
//!
//! The data field is intended to be variable-length and will contain the F Prime payload (e.g. a command buffer)
typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader;
    uint8 data[FPRIME_RAW_FRAME_MAX_SIZE];
} FPRIME_FprimeCommandMessage_t;

//! \brief a generic telemetry message for F Prime telemetry and downlink data
//!
//! This structure is used to encapsulate F Prime telemetry and downlink data in a format compatible with the cFS
//! software bus. It has the appropriate header and a data field large enough to hold all F Prime downlink data.
//!
//! The data field is intended to be variable-length and will contain the F Prime payload (e.g. a telemetry buffer)
typedef struct
{
    CFE_MSG_TelemetryHeader_t  TelemetryHeader;
    uint8 data[FPRIME_RAW_FRAME_MAX_SIZE];
} FPRIME_FprimeTelemetryMessage_t;

//! \brief a union of the generic F Prime command and telemetry message types
//!
//! This is used as a convenience for handling the type of messages being transmitted.
typedef union {
    FPRIME_FprimeCommandMessage_t command;
    FPRIME_FprimeTelemetryMessage_t telemetry;
} FPRIME_FprimeMessage_t;

#if defined(__cplusplus)
}  // extern "C"
#endif

#endif