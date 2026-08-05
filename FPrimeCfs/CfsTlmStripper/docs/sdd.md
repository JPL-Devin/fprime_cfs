# FPrimeCfs::CfsTlmStripper

A framer stage that strips the cFS telemetry secondary header (big-endian 4-byte seconds, 2-byte subseconds)
from complete telemetry space packets, rebuilding the CCSDS primary header. It is the inverse of
`FPrimeCfs.CfsTlmFramer`, intended for ground-facing bridges (e.g. a GDS bridge application) whose downstream
consumers expect bare F Prime packets inside the space packets.

## Usage

```
FPrimeCfs.CfsBridge.dataOut -> CfsTlmStripper.dataIn
CfsTlmStripper.dataOut -> [downlink framer, e.g. Svc.Ccsds.TmFramer]
[downlink framer].dataReturnOut -> CfsTlmStripper.dataReturnIn
CfsTlmStripper.dataReturnOut -> FPrimeCfs.CfsBridge.dataReturnIn
CfsTlmStripper.bufferAllocate / bufferDeallocate -> [buffer manager]
```

## Behavior

For each incoming buffer holding a complete space packet:

- Telemetry packets (packet type bit clear) with the secondary header flag set have the 6-byte cFS telemetry
  secondary header removed. The primary header is rebuilt: the packet data length field shrinks by 6 and the
  secondary header flag is cleared. The forwarded context has `hasSecHdr = false`.
- All other packets (commands, packets without the secondary header flag) are forwarded unchanged.

The output is always an allocated copy, so upstream producers operating in zero-copy mode (e.g.
`FPrimeCfs.CfsBridge` polling the software bus without an allocator) are supported: the incoming buffer is
returned to its sender on `dataReturnOut` before the handler returns.

Buffers too small to hold a primary header are dropped with a `MalformedPacket` warning event. Allocation
failures drop the packet with an `AllocationFailed` warning event.

## Buffer ownership

- `dataIn`: a new buffer is allocated via `bufferAllocate` for the outgoing packet; the incoming buffer is
  returned to the sender on `dataReturnOut` immediately with its original context.
- `dataReturnIn`: the allocated buffer coming back from downstream is deallocated via `bufferDeallocate`.

`comStatusIn` is passed through to `comStatusOut` unchanged.
