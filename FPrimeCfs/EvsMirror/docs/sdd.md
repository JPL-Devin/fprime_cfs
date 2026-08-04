# FPrimeCfs::EvsMirror

Mirror F Prime events to the cFS Event Services (EVS) subsystem. `EvsMirror` is a passive
pass-through component that sits between event-producing components and the `Svc::EventManager`:
events received on `logIn` are forwarded unchanged on `logOut`, and text events received on
`textLogIn` are forwarded unchanged on `textLogOut`. In addition, each event is mirrored onto the
cFS event subsystem with `CFE_EVS_SendEvent()`, making F Prime events visible to native cFS ground
tooling alongside events from other cFS applications.

When text logging is enabled (`FW_ENABLE_TEXT_LOGGING`, the default), the mirror uses the fully
formatted event text arriving on `textLogIn`. When text logging is disabled, the formatted text is
unavailable on board, so the mirror instead emits a compact identifier form
(`F Prime EVR 0x<id> severity <n>`) from `logIn`; ground systems can resolve the ID via the F Prime
dictionary. In either build exactly one EVS event is emitted per F Prime event.

## Usage Examples

Instantiate the mirror between components and the event manager in the topology:

```fpp
instance evsMirror: FPrimeCfs.EvsMirror base id 0x2000

connections Events {
    component.log -> evsMirror.logIn
    evsMirror.logOut -> eventLogger.LogRecv
    component.logText -> evsMirror.textLogIn
    evsMirror.textLogOut -> textLogger.TextLogger
}
```

The hosting cFS application must register with EVS during initialization, as with any cFS app:

```cpp
CFE_Status_t status = CFE_EVS_Register(nullptr, 0, CFE_EVS_EventFilter_BINARY);
```

## Port Descriptions

| Kind | Name | Type | Description |
|---|---|---|---|
| sync input | `logIn` | `Fw.Log` | Event input; forwarded unchanged on `logOut`. Mirrored to EVS in compact form when text logging is disabled |
| output | `logOut` | `Fw.Log` | Forwarded events; connect to the `Svc.EventManager` event input |
| sync input | `textLogIn` | `Fw.LogText` | Text event input; forwarded unchanged on `textLogOut`. Mirrored to EVS with the formatted text when text logging is enabled |
| output | `textLogOut` | `Fw.LogText` | Forwarded text events; connect to a text logger (e.g. `Svc.PassiveTextLogger`) if desired |

## Requirements

| Name | Description | Validation |
|---|---|---|
| FPRIMECFS-EVSMIRROR-001 | `EvsMirror` shall forward every event received on `logIn` unchanged on `logOut` | Unit test |
| FPRIMECFS-EVSMIRROR-002 | `EvsMirror` shall forward every text event received on `textLogIn` unchanged on `textLogOut` | Unit test |
| FPRIMECFS-EVSMIRROR-003 | `EvsMirror` shall mirror each event to EVS exactly once via `CFE_EVS_SendEvent()`, using the formatted event text when text logging is enabled and a compact identifier form otherwise, with the EVS event ID set to the low 16 bits of the F Prime event ID | Unit test |
| FPRIMECFS-EVSMIRROR-004 | `EvsMirror` shall map F Prime event severities to EVS event types: FATAL to CRITICAL; WARNING_HI and WARNING_LO to ERROR; COMMAND, ACTIVITY_HI, and ACTIVITY_LO to INFORMATION; DIAGNOSTIC to DEBUG | Unit test |
| FPRIMECFS-EVSMIRROR-005 | `EvsMirror` shall log a console error and continue forwarding when `CFE_EVS_SendEvent()` fails, without asserting | Unit test |

## Design

`EvsMirror` is passive: both input ports are synchronous, so mirroring and forwarding execute on
the caller's thread with no queueing, adding minimal latency to the event path.

The mirror always passes the event text to `CFE_EVS_SendEvent()` through a `"%s"` format
specifier, so `%` characters in event text are never interpreted as format directives. EVS event
IDs are 16 bits wide; the F Prime event ID is truncated to its low 16 bits. EVS itself truncates
event text longer than `CFE_MISSION_EVS_MAX_MESSAGE_LENGTH`. Note that EVS applies its own
filtering, so mirrored events are additionally subject to the application's EVS filter settings.

The `FW_ENABLE_TEXT_LOGGING` switch selects which input port performs the mirror (text form on
`textLogIn` when enabled, compact form on `logIn` when disabled), guaranteeing one EVS event per
F Prime event in either configuration.

## Unit Testing

Unit tests run standalone (no cFS build) against a stub cFE EVS layer in `test/ut/stubs/` that
records `CFE_EVS_SendEvent()` calls (ID, type, and formatted text) and allows injection of error
statuses. The suite covers pass-through of both ports, text mirroring, event ID truncation,
severity mapping, and the EVS failure path.

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
