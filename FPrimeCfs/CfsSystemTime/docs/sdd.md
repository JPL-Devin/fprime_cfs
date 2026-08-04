# FPrimeCfs::CfsSystemTime

A time component backed by cFS time services. `CfsSystemTime` implements the standard `Svc.Time`
interface: each call to `timeGetPort` returns an `Fw::Time` constructed from the seconds and
subseconds reported by `CFE_TIME_GetTime`, making cFS spacecraft time available to every component
in an F Prime topology through the standard `time get` port.

The component also provides a `cfsTimeConvert` port that performs the reverse conversion: it takes
an `Fw::Time` and returns an `FPrimeCfs::CfsTime` (a mirror of `CFE_TIME_SysTime_t`), allowing
components to translate F Prime timestamps back into cFS system time — for example when populating
a cFS telemetry secondary header.

`CfsSystemTime` is a **passive** component: all conversions execute on the caller's thread.

## Time Representation

cFS represents fractional seconds in 2^-32 second subsecond units, while `Fw::Time` uses
microseconds. Conversions use the cFE-provided `CFE_TIME_Sub2MicroSecs` and
`CFE_TIME_Micro2SubSecs` functions.

## Port Descriptions

| Kind         | Name             | Type                       | Description                                                        |
|--------------|------------------|----------------------------|--------------------------------------------------------------------|
| `sync input` | `timeGetPort`    | `Fw.Time`                  | Standard time interface: returns the current cFS time as `Fw.Time` |
| `sync input` | `cfsTimeConvert` | `FPrimeCfs.CfsTimeConvert` | Converts an `Fw.Time` back to an `FPrimeCfs.CfsTime`               |

## Usage Examples

```
# In the topology's instance definitions
instance cfsTime: FPrimeCfs.CfsSystemTime base id 0x4000

# In the topology
time connections instance cfsTime
```

## Requirements

| Name                | Description                                                                                    | Validation |
|---------------------|------------------------------------------------------------------------------------------------|------------|
| FPRIME-CFS-TIME-001 | `CfsSystemTime` shall return `Fw::Time` objects built from `CFE_TIME_GetTime` seconds/subseconds | Unit test  |
| FPRIME-CFS-TIME-002 | `CfsSystemTime` shall convert `Fw::Time` objects to `FPrimeCfs::CfsTime` on `cfsTimeConvert`      | Unit test  |
