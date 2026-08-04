# FPrimeCfs::EvsMirror

Publish F Prime events to the cFS Event Services (EVS) subsystem. `EvsMirror` is a passive
drop-in replacement for `Svc::PassiveTextLogger` on the text event (text log) branch: component
text event outputs connect to its `TextLogger` port, and each formatted event is published with
`CFE_EVS_SendEvent()`. cFS handles the text logging piece from there — EVS writes events to the
console and distributes them to the ground like any other cFS application event.

## Usage Examples

Instantiate in place of the standard text logger in the topology:

```fpp
instance textLogger: FPrimeCfs.EvsMirror base id 0x2000
```

Component text event ports route to it through the standard topology text event pattern; no other
connections are required. The hosting cFS application must register with EVS during
initialization, as with any cFS app:

```cpp
CFE_Status_t status = CFE_EVS_Register(nullptr, 0, CFE_EVS_EventFilter_BINARY);
```

Text logging (`FW_ENABLE_TEXT_LOGGING`, on by default) must be enabled for components to emit
text events.

## Port Descriptions

| Kind | Name | Type | Description |
|---|---|---|---|
| sync input | `TextLogger` | `Fw.LogText` | Text event input; each event is published to EVS via `CFE_EVS_SendEvent()` |

## Requirements

| Name | Description | Validation |
|---|---|---|
| FPRIMECFS-EVSMIRROR-001 | `EvsMirror` shall publish each text event received on `TextLogger` to EVS exactly once via `CFE_EVS_SendEvent()`, using the formatted event text, with the EVS event ID set to the low 16 bits of the F Prime event ID | Unit test |
| FPRIMECFS-EVSMIRROR-002 | `EvsMirror` shall map F Prime event severities to EVS event types: FATAL to CRITICAL; WARNING_HI and WARNING_LO to ERROR; COMMAND, ACTIVITY_HI, and ACTIVITY_LO to INFORMATION; DIAGNOSTIC to DEBUG | Unit test |
| FPRIMECFS-EVSMIRROR-003 | `EvsMirror` shall log a console error and continue when `CFE_EVS_SendEvent()` fails, without asserting | Unit test |

## Design

`EvsMirror` is passive with a single synchronous input port, so publishing executes on the
caller's thread with no queueing — the same execution profile as `Svc::PassiveTextLogger`.

The event text is passed to `CFE_EVS_SendEvent()` through a `"%s"` format specifier, so `%`
characters in event text are never interpreted as format directives. EVS event IDs are 16 bits
wide; the F Prime event ID is truncated to its low 16 bits. EVS itself truncates event text longer
than `CFE_MISSION_EVS_MAX_MESSAGE_LENGTH`. Note that EVS applies its own filtering, so published
events are subject to the application's EVS filter settings.

## Unit Testing

Unit tests run standalone (no cFS build) against a stub cFE EVS layer in `test/ut/stubs/` that
records `CFE_EVS_SendEvent()` calls (ID, type, and formatted text) and allows injection of error
statuses. The suite covers publishing and event ID truncation, severity mapping, and the EVS
failure path.

To run:

```bash
fprime-util generate --ut
fprime-util build --ut -j"$(nproc)"
fprime-util check
```

## Change Log

| Date | Description |
|---|---|
| 2026-08-04 | Initial SDD with requirements and unit tests |
| 2026-08-04 | Restructure as a drop-in replacement for the text logger: single `TextLogger` input, no pass-through ports; cFS handles console output |
