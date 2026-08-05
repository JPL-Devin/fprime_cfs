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
```

## Behavior

For each incoming buffer holding a complete space packet:

- Telemetry packets (packet type bit clear) with the secondary header flag set have the 6-byte cFS telemetry
  secondary header removed **in place**: the primary header is rewritten into the 6 bytes immediately preceding
  the payload (over the secondary header), the packet data length field shrinks by 6, the secondary header flag
  is cleared, and the buffer's data pointer is advanced by 6 with the size reduced accordingly. No allocation
  or payload copy is performed. The forwarded context has `hasSecHdr = false`.
- All other packets (commands, packets without the secondary header flag) are forwarded unchanged.

Buffers too small to hold a primary header are dropped with a `MalformedPacket` warning event and returned to
the sender.

## Buffer ownership

- `dataIn`: the incoming buffer itself (with an adjusted data pointer for stripped packets) is forwarded on
  `dataOut`; ownership transfers downstream.
- `dataReturnIn`: the buffer coming back from downstream is passed through to `dataReturnOut`, returning
  ownership to the original sender. Allocator-owned buffers (e.g. `Svc.BufferManager`) are tracked through the
  buffer context word, so the advanced data pointer does not affect their return.

`comStatusIn` is passed through to `comStatusOut` unchanged.

## Requirements

| Requirement | Description | Verification Method |
|---|---|---|
| FPRIMECFS-CFSTLMSTRIPPER-001 | CfsTlmStripper shall remove the 6-byte cFS telemetry secondary header from each telemetry space packet (packet type bit clear) received on `dataIn` whose secondary header flag is set, forwarding the stripped packet on `dataOut` with `hasSecHdr = false` in the context. | Unit test |
| FPRIMECFS-CFSTLMSTRIPPER-002 | For stripped packets, CfsTlmStripper shall rebuild the CCSDS primary header such that the secondary header flag is cleared, the packet data length field is reduced by the secondary header size, and all other primary header fields are preserved. | Unit test |
| FPRIMECFS-CFSTLMSTRIPPER-003 | CfsTlmStripper shall strip packets in place, without allocating a new buffer or copying the packet payload, by advancing the buffer's data pointer past the removed header bytes. | Unit test |
| FPRIMECFS-CFSTLMSTRIPPER-004 | CfsTlmStripper shall forward command packets, and packets whose secondary header flag is clear, on `dataOut` unchanged. | Unit test |
| FPRIMECFS-CFSTLMSTRIPPER-005 | CfsTlmStripper shall drop buffers too small to hold a CCSDS primary header, emitting a warning event and returning the buffer to the sender on `dataReturnOut`. | Unit test |
| FPRIMECFS-CFSTLMSTRIPPER-006 | CfsTlmStripper shall return every buffer received on `dataReturnIn` to the sender on `dataReturnOut` with the context it was received with. | Unit test |
| FPRIMECFS-CFSTLMSTRIPPER-007 | CfsTlmStripper shall pass communication status received on `comStatusIn` through to `comStatusOut` unchanged. | Unit test |
