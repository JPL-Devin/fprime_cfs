# FPrimeCfs::CfsTlmDeframer

Removes the cFS telemetry secondary header from received telemetry space packets. `CfsTlmDeframer` sits between a
source of complete cFS messages (e.g. a `FPrimeCfs.CfsBridge` receiving from the cFS software bus) and a consumer
expecting bare F Prime space packets (e.g. an `Svc.Ccsds.TmFramer` framing packets for the F Prime GDS). Each
telemetry packet carrying a secondary header is rewritten in place:

- the secondary header flag is cleared in the CCSDS primary header,
- the primary header length field is decreased by the secondary header size (6 bytes), and
- the 6-byte cFS telemetry secondary header is removed by shifting the payload forward.

Command packets and telemetry packets without a secondary header pass through unmodified. The packet payload is
preserved byte-for-byte in all cases.

## Usage

```
[SB source].dataOut          -> cfsTlmDeframer.dataIn
cfsTlmDeframer.dataReturnOut -> [SB source].dataReturnIn
cfsTlmDeframer.dataOut       -> [downstream framer].dataIn
[downstream framer].dataReturnOut -> cfsTlmDeframer.dataReturnIn
```

The `CfsTlmFraming.Deframing` subtopology (`FPrimeCfs/Subtopologies/CfsTlmFraming`) boxes an instance of this
component with topology ports matching the connections above.

## Behavior

- `dataIn` receives a buffer holding a single complete space packet. Stripping is performed in place: no buffer
  allocation occurs, and the buffer emitted on `dataOut` is the input buffer (possibly shortened by 6 bytes).
- A telemetry packet whose secondary header flag is set but whose structure is too short or inconsistent to
  strip safely is forwarded unmodified with a logged warning.
- Buffers coming back on `dataReturnIn` are returned upstream on `dataReturnOut`, completing the ownership
  transfer back to the original sender.
- The `FrameContext` is passed through unmodified; packet headers are authoritative for downstream consumers.

## Requirements

| Requirement | Description | Verification |
|---|---|---|
| FPRIMECFS-CFSTLMDEFRAMER-001 | The component shall strip the 6-byte cFS telemetry secondary header from telemetry packets carrying one (flag cleared, length field adjusted, payload shifted in place). | Unit test |
| FPRIMECFS-CFSTLMDEFRAMER-002 | The component shall preserve the packet payload byte-for-byte. | Unit test |
| FPRIMECFS-CFSTLMDEFRAMER-003 | The component shall pass command packets and telemetry packets without a secondary header through unmodified. | Unit test |
| FPRIMECFS-CFSTLMDEFRAMER-004 | The component shall forward packets too short or inconsistent to strip safely unmodified. | Unit test |
| FPRIMECFS-CFSTLMDEFRAMER-005 | The component shall return buffers received on `dataReturnIn` upstream on `dataReturnOut`. | Unit test |
