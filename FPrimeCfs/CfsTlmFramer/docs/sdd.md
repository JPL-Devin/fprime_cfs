# FPrimeCfs::CfsTlmFramer

Frames downlinked F Prime telemetry space packets as valid cFS telemetry packets. `CfsTlmFramer` sits between a
source of complete CCSDS space packets (e.g. an `Svc.Ccsds.SpacePacketFramer`, possibly aggregated by an
`Svc.ComAggregator`) and a consumer that transmits them on the cFS software bus (e.g. a `FPrimeCfs.CfsBridge`).
Each telemetry packet without a secondary header is rewritten as a valid cFS telemetry packet:

- the secondary header flag is set in the CCSDS primary header,
- the primary header length field is increased by the secondary header size (6 bytes), and
- the 6-byte cFS telemetry secondary header — big-endian 4-byte seconds followed by 2-byte subseconds
  (units of 2^-16 seconds), taken from the component's time source — is inserted between the primary header
  and the packet payload.

Command packets and packets already carrying a secondary header pass through unmodified. The packet payload is
preserved byte-for-byte in all cases.

## Usage

```
[packet source].dataOut     -> cfsTlmFramer.dataIn
cfsTlmFramer.dataReturnOut  -> [packet source].dataReturnIn
cfsTlmFramer.comStatusOut   -> [packet source].comStatusIn
cfsTlmFramer.dataOut        -> [SB consumer].dataIn
[SB consumer].dataReturnOut -> cfsTlmFramer.dataReturnIn
[SB consumer].comStatusOut  -> cfsTlmFramer.comStatusIn
cfsTlmFramer.bufferAllocate   -> [buffer manager].bufferGetCallee
cfsTlmFramer.bufferDeallocate -> [buffer manager].bufferSendIn
```

The `CfsTlmFraming.Framing` subtopology (`FPrimeCfs/Subtopologies/CfsTlmFraming`) boxes an instance of this
component with topology ports matching the connections above.

## Behavior

- `dataIn` receives a buffer holding one or more complete space packets. The component validates the packet
  structure, allocates an output buffer sized for the input plus one 6-byte secondary header per telemetry
  packet lacking one, copies each packet (inserting secondary headers where needed), and emits the result on
  `dataOut`. The original buffer is returned upstream on `dataReturnOut`.
- A structurally malformed buffer (truncated packet or header) is forwarded verbatim with a logged warning;
  the downstream consumer performs its own validation.
- If the output buffer allocation fails, the input buffer is dropped with a logged error and returned upstream,
  and a success com status is emitted so upstream queueing continues to flow.
- Buffers coming back on `dataReturnIn` (previously emitted on `dataOut`) are deallocated via
  `bufferDeallocate`.
- Com status signals pass through from `comStatusIn` to `comStatusOut`.
- The `FrameContext` is passed through unmodified; packet headers are authoritative for downstream consumers.

## Requirements

| Requirement | Description | Verification |
|---|---|---|
| FPRIMECFS-CFSTLMFRAMER-001 | The component shall rewrite telemetry space packets without a secondary header as valid cFS telemetry packets (secondary header flag set, length field adjusted, 6-byte time secondary header inserted). | Unit test |
| FPRIMECFS-CFSTLMFRAMER-002 | The component shall fill the telemetry secondary header with the current time as big-endian 4-byte seconds and 2-byte subseconds (2^-16 s). | Unit test |
| FPRIMECFS-CFSTLMFRAMER-003 | The component shall pass command packets and packets already carrying a secondary header through unmodified. | Unit test |
| FPRIMECFS-CFSTLMFRAMER-004 | The component shall handle buffers holding multiple concatenated space packets, framing each qualifying packet. | Unit test |
| FPRIMECFS-CFSTLMFRAMER-005 | The component shall forward structurally malformed buffers verbatim. | Unit test |
| FPRIMECFS-CFSTLMFRAMER-006 | Upon allocation failure the component shall return the input buffer upstream and emit a success com status. | Unit test |
| FPRIMECFS-CFSTLMFRAMER-007 | The component shall deallocate buffers returned on `dataReturnIn`. | Unit test |
| FPRIMECFS-CFSTLMFRAMER-008 | The component shall pass com status signals through. | Unit test |
