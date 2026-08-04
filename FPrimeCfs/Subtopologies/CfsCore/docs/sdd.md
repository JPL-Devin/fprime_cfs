# CfsCore Subtopology

The CfsCore subtopology boxes the core command and data handling stack of an F Prime application hosted as a cFS
app. It is modeled after the F Prime `CdhCore` subtopology with cFS-specific choices:

- Text logging is replaced by the `FPrimeCfs.EvsMirror`, which mirrors F Prime events to the cFS Event Services
  (EVS) subsystem.
- Telemetry is sent through the `Svc.TlmPacketizer` (rather than `Svc.TlmChan`), matching the packetized downlink
  of the cFS communications stack.
- Time is provided by the `FPrimeCfs.CfsSystemTime` component, backed by cFS time services (`CFE_TIME`).
- The `FPrimeCfs.SchAppDriver` converts cFS scheduler (SCH) messages into rate group cycles.
- The `Svc.EventManager` is optional: import `CfsCore.Subtopology` to omit it (events go to EVS only, through the
  text logging connections to `evsMirror`), or `CfsCore.SubtopologyWithEvents` to include it (event packets are
  also downlinked as F Prime event packets).

## Instances

| Instance | Component | Notes |
|---|---|---|
| `cmdDisp` | `Svc.CommandDispatcher` | Command dispatch |
| `tlmSend` | `Svc.TlmPacketizer` | Defined in `CfsCoreConfig/CfsCoreTlmConfig.fpp`; projects override to set their packet list |
| `$health` | `Svc.Health` | Ping-based health monitoring |
| `version` | `Svc.Version` | Version reporting |
| `evsMirror` | `FPrimeCfs.EvsMirror` | Text logger replacement |
| `fatalAdapter` | `Svc.AssertFatalAdapter` | Assert-to-FATAL adapter |
| `fatalHandler` | `Svc.FatalHandler` | Defined in `CfsCoreConfig/CfsCoreFatalHandlerConfig.fpp` |
| `schAppDriver` | `FPrimeCfs.SchAppDriver` | cFS scheduler-driven rate group driver |
| `cfsTime` | `FPrimeCfs.CfsSystemTime` | cFS-backed time provider |
| `events` | `Svc.EventManager` | Optional; `SubtopologyWithEvents` only, defined in `CfsCoreConfig/CfsCoreEventsConfig.fpp` |

## Pattern connections

Deployments using this subtopology declare the pattern connections:

```
command connections instance CfsCore.cmdDisp
telemetry connections instance CfsCore.tlmSend
text event connections instance CfsCore.evsMirror
health connections instance CfsCore.$health
time connections instance CfsCore.cfsTime
event connections instance CfsCore.events   # SubtopologyWithEvents only
```

## Exported ports

| Port | Topology | Direction | Description |
|---|---|---|---|
| `seqCmdBuff` | `Subtopology` | in | Command buffers into the command dispatcher |
| `seqCmdStatus` | `Subtopology` | out | Command execution status back to the command source |
| `tlmSendPktSend` | `Subtopology` | out (array) | Packetized telemetry to the comm stack |
| `tlmSendRun` | `Subtopology` | in | Telemetry packet send cycle |
| `cmdDispRun` | `Subtopology` | in | Command dispatcher scheduling |
| `healthRun` | `Subtopology` | in | Health component scheduling |
| `cfsCommandIn` | `Subtopology` | in | Routed cFS scheduler messages into the SchAppDriver |
| `schBufferReturnOut` | `Subtopology` | out | Ownership return for buffers received on `cfsCommandIn` |
| `cycleOut` | `Subtopology` | out | One tick per scheduler message (connect to a `Svc.RateGroupDriver`) |
| `cfsTimeConvert` | `Subtopology` | in | F Prime to cFS time conversion |
| `fatalReceive` | `Subtopology` | in | FATAL announcements into the fatal handler |
| `eventsPktSend` | `SubtopologyWithEvents` | out | Event packets from the EventManager |
| `eventsRun` | `SubtopologyWithEvents` | in | EventManager scheduling |

## Fatal handling

`SubtopologyWithEvents` connects `events.FatalAnnounce` to `fatalHandler.FatalReceive` internally. The base
`Subtopology` omits the `Svc.EventManager`, so deployments using it must route their FATAL announcement source
(e.g. their own event manager or fatal announcer) to the exported `fatalReceive` port; otherwise FATAL events
will not reach the fatal handler.

## Configuration

`CfsCoreConfig/CfsCoreConfig.fpp` defines the base ID, queue sizes, stack sizes, and priorities. The
`Svc.TlmPacketizer` requires a packet list to be set before use; projects override
`CfsCoreConfig/CfsCoreTlmConfig.fpp` (via `register_fprime_config` `CONFIGURATION_OVERRIDES`) to add a
`configComponents` phase calling `setPacketList` with their deployment's generated packet list. See the notes in
that file for an example. The default (unoverridden) configuration sets an empty packet list so that a deployment
missing the override degrades gracefully (no telemetry packets downlinked) instead of asserting on the first
telemetry write.
