# CfsCore Subtopology

The CfsCore subtopology boxes the core command and data handling stack of an F Prime application hosted as a cFS
app. It is modeled after the F Prime `CdhCore` subtopology with cFS-specific choices:

- Text logging is replaced by the `FPrimeCfs.EvsMirror`, which mirrors F Prime events to the cFS Event Services
  (EVS) subsystem.
- Telemetry is sent through the `Svc.TlmPacketizer` (rather than `Svc.TlmChan`), matching the packetized downlink
  of the cFS communications stack.
- Time is provided by the `FPrimeCfs.CfsSystemTime` component, backed by cFS time services (`CFE_TIME`).
- The `FPrimeCfs.SchAppDriver` converts cFS scheduler (SCH) messages into rate group cycles.
- The `Svc.EventManager` downlinks events as F Prime event packets, in addition to the EVS mirroring provided by
  `evsMirror`.

## Instances

| Instance | Component | Notes |
|---|---|---|
| `cmdDisp` | `Svc.CommandDispatcher` | Command dispatch |
| `tlmSend` | `Svc.TlmPacketizer` | Packetized telemetry; deployments set their generated packet list |
| `events` | `Svc.EventManager` | Event packet downlink |
| `$health` | `Svc.Health` | Ping-based health monitoring |
| `version` | `Svc.Version` | Version reporting |
| `evsMirror` | `FPrimeCfs.EvsMirror` | Text logger replacement |
| `fatalAdapter` | `Svc.AssertFatalAdapter` | Assert-to-FATAL adapter |
| `fatalHandler` | `Svc.FatalHandler` | Defined in `CfsCoreConfig/CfsCoreFatalHandlerConfig.fpp` |
| `schAppDriver` | `FPrimeCfs.SchAppDriver` | cFS scheduler-driven rate group driver |
| `cfsTime` | `FPrimeCfs.CfsSystemTime` | cFS-backed time provider |

## Pattern connections

Deployments using this subtopology declare the pattern connections:

```
command connections instance CfsCore.cmdDisp
telemetry connections instance CfsCore.tlmSend
text event connections instance CfsCore.evsMirror
health connections instance CfsCore.$health
time connections instance CfsCore.cfsTime
event connections instance CfsCore.events
```

## Exported ports

| Port | Direction | Description |
|---|---|---|
| `seqCmdBuff` | in | Command buffers into the command dispatcher |
| `seqCmdStatus` | out | Command execution status back to the command source |
| `tlmSendPktSend` | out (array) | Packetized telemetry to the comm stack |
| `tlmSendRun` | in | Telemetry packet send cycle |
| `cmdDispRun` | in | Command dispatcher scheduling |
| `healthRun` | in | Health component scheduling |
| `cfsCommandIn` | in | Routed cFS scheduler messages into the SchAppDriver |
| `schBufferReturnOut` | out | Ownership return for buffers received on `cfsCommandIn` |
| `cycleOut` | out | One tick per scheduler message (connect to a `Svc.RateGroupDriver`) |
| `cfsTimeConvert` | in | F Prime to cFS time conversion |
| `eventsPktSend` | out | Event packets from the EventManager |
| `eventsRun` | in | EventManager scheduling |

## Fatal handling

The subtopology connects `events.FatalAnnounce` to `fatalHandler.FatalReceive` internally.

## Configuration

`CfsCoreConfig/CfsCoreConfig.fpp` defines the base ID, queue sizes, stack sizes, and priorities. The
`Svc.TlmPacketizer` requires a packet list to be set before use; deployments call `setPacketList` with their
generated packet list during component configuration. The default configuration sets an empty packet list so that
a deployment that has not yet done so degrades gracefully (no telemetry packets downlinked) instead of asserting
on the first telemetry write.
