/**
 * @file
 *   Minimal cFE message header stub for CfsBridge unit testing.
 *
 *   The primary header is CCSDS binary-accurate (6 bytes: stream id, sequence,
 *   and length fields) since the code under test parses and forwards complete
 *   CCSDS space packets. Secondary headers use representative sizes only.
 */
#ifndef FPRIME_CFS_UT_STUB_CFE_MSG_HDR_H
#define FPRIME_CFS_UT_STUB_CFE_MSG_HDR_H

#include <stddef.h>
#include <stdint.h>

#if defined(__cplusplus)
extern "C" {
#endif

/* CCSDS space packet primary header: 6 big-endian bytes */
typedef struct {
    uint8_t StreamId[2];
    uint8_t Sequence[2];
    uint8_t Length[2];
} CCSDS_PrimaryHeader_t;

typedef struct {
    CCSDS_PrimaryHeader_t Pri;
} CFE_MSG_Message_t;

typedef struct {
    CFE_MSG_Message_t Msg;
    uint8_t Sec[2];
} CFE_MSG_CommandHeader_t;

typedef struct {
    CFE_MSG_Message_t Msg;
    uint8_t Sec[6];
} CFE_MSG_TelemetryHeader_t;

#if defined(__cplusplus)
}
#endif

#endif
