# FPrimeCfs::CfsCmdRouter

Routes cFS command messages to consumers based on the function code in the cFS command secondary header.
`CfsCmdRouter` is instantiated after a `CfsRouter` (which routes APIDs to consumers): the incoming data is a
cFS command message without its space packet primary header, i.e. it starts at the cFS command secondary header
(`{U8 FunctionCode, U8 Checksum}`) followed by the command payload. The router first validates the cFS command
checksum, dropping the message with a warning event on failure, and then routes the message by function code
using a static configuration table just like the `CfsRouter` APID routing table.

## Usage

```
CfsRouter.unknownDataOut (or any Svc.ComDataWithContext source) -> CfsCmdRouter.dataIn
CfsCmdRouter.dataReturnOut -> [sender's data return input]
CfsCmdRouter.comOut[i] -> [command dispatcher].seqCmdBuff
CfsCmdRouter.bufferOut[j] / unknownDataOut -> [function-code-aware consumers]
[consumers' buffer return] -> CfsCmdRouter.bufferReturnIn
```

Routing is selected by a static function code routing table configured in
`config/CfsCmdRouterConfig/CfsCmdRouterCfg.fpp`. Projects override this file (via `register_fprime_config`
`CONFIGURATION_OVERRIDES`) to supply their own table, table size, port-array sizes, and pending-buffer limit:

```
constant CFS_CMD_ROUTER_ROUTE_TABLE_SIZE = 3
constant CFS_CMD_ROUTER_ROUTE_TABLE = [
    { functionCode = 0, routeType = CfsCmdRouteType.COM,    portIndex = 0 },
    { functionCode = 1, routeType = CfsCmdRouteType.BUFFER, portIndex = 0 },
    { functionCode = 2, routeType = CfsCmdRouteType.BUFFER, portIndex = 1 },
]
```

The route entry type (`CfsCmdRouteEntry`) and route type enumeration (`CfsCmdRouteType`) are defined in the
`Types` module (`Types/CfsCmdRouterTypes.fpp`); the `CfsCommand` port type used by the buffer route is defined
in the common `FPrimeCfs/Ports` module.

## Route types

- `COM` ("bare com"): the message without the 2-byte secondary header is copied into an `Fw::ComBuffer` and
  sent on the configured `comOut` index when that port is connected (e.g. for an F Prime command dispatcher).
  The incoming buffer is returned to the sender immediately. This is the only route that copies.
- `BUFFER`: the payload (`Fw::Buffer` without the secondary header) is sent on the configured `bufferOut` index
  together with the function code as a separate argument (the `FPrimeCfs.CfsCommand` port type). Ownership of
  the buffer transfers to the receiver and returns on `bufferReturnIn`.
- Function codes not present in the table route to `unknownDataOut` with their context; ownership transfers to
  the receiver and returns on `bufferReturnIn`.

## Checksum validation

The cFS command checksum is validated per CFE_MSG conventions: the XOR of every byte of the complete space
packet with `0xFF` must equal zero. Because the space packet primary header is consumed upstream (by the
deframer), the router reconstructs it from the frame context, assuming a cFS command packet: packet version
number 0, packet type command (1), secondary header present (1), the context APID, sequence flags unsegmented
(`0b11`), the context sequence count, and the length field derived from the data size. Messages failing the
checksum are dropped: a warning event is emitted with the XOR residual and the buffer is returned to the sender.

Messages whose context indicates no secondary header, or whose data is too small to contain one, cannot carry a
function code or checksum and are routed to the unknown output with a warning event (mirroring `CfsRouter`).

## Buffer ownership

- `COM` route: the message data is copied; the incoming buffer is returned on `dataReturnOut` immediately.
- `BUFFER` and unknown routes: ownership of the (payload) buffer transfers to the receiver, and returns on
  `bufferReturnIn`, which forwards it to `dataReturnOut`.
- Buffers are always returned on `dataReturnOut` with the context they were received with. For pass-through
  routes the router records the buffer-to-context association in a map (capacity
  `CFS_CMD_ROUTER_MAX_PENDING_BUFFERS`); if the map is full, the message is returned unrouted with a warning
  event.
- If the target output port for a computed route is not connected, the buffer is returned immediately rather
  than asserting or leaking.

## Requirements

| Requirement | Description | Verification Method |
|---|---|---|
| FPRIMECFS-CFSCMDROUTER-001 | CfsCmdRouter shall select a route for each message received on `dataIn` using a statically configured function code to (route type, port index) routing table. | Unit test |
| FPRIMECFS-CFSCMDROUTER-002 | CfsCmdRouter shall validate the cFS command checksum of each message before routing, and shall drop messages failing validation, returning the buffer to the sender and emitting a warning event. | Unit test |
| FPRIMECFS-CFSCMDROUTER-003 | CfsCmdRouter shall route messages whose function code is configured as `COM` to the configured com output port, when connected, by copying the message without the secondary header into an `Fw::ComBuffer`. | Unit test |
| FPRIMECFS-CFSCMDROUTER-004 | For `COM` routing, CfsCmdRouter shall return ownership of the received buffer to the sender immediately after copying. | Unit test |
| FPRIMECFS-CFSCMDROUTER-005 | CfsCmdRouter shall route messages whose function code is configured as `BUFFER` to the configured buffer output port, providing the function code (`U8`) as a separate argument and the payload buffer without the secondary header. | Unit test |
| FPRIMECFS-CFSCMDROUTER-006 | CfsCmdRouter shall route any message whose function code is not present in the routing table to the unknown data output port with its context. | Unit test |
| FPRIMECFS-CFSCMDROUTER-007 | CfsCmdRouter shall route messages whose context indicates no secondary header, or whose data is too small to contain the secondary header, to the unknown data output port and emit a warning event. | Unit test |
| FPRIMECFS-CFSCMDROUTER-008 | For `BUFFER` and unknown routing, CfsCmdRouter shall transfer buffer ownership to the receiver and return ownership to the sender only after the buffer is returned on `bufferReturnIn`. | Unit test |
| FPRIMECFS-CFSCMDROUTER-009 | CfsCmdRouter shall complete every buffer ownership transfer by eventually returning the received buffer to the sender on `dataReturnOut` with the context it was received with. | Unit test |
| FPRIMECFS-CFSCMDROUTER-010 | CfsCmdRouter shall emit a warning-severity event when message data cannot be copied into a com buffer. | Unit test |
| FPRIMECFS-CFSCMDROUTER-011 | When the target output port for a computed route is not connected, CfsCmdRouter shall return the received buffer to the sender rather than assert or leak. | Unit test |
| FPRIMECFS-CFSCMDROUTER-012 | CfsCmdRouter shall accept command responses on `cmdResponseIn` as a no-op. | Unit test |
