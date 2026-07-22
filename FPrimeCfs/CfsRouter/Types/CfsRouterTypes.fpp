#####
# CfsRouter types:
#
# Types for the CfsRouter
#####

module FPrimeCfs {

    @ The category of output port that an APID routes to
    enum CfsRouteType : U8 {
        FPRIME_COMMAND  @< Route to an F Prime command output port (Fw.Com)
        CFS_COMMAND     @< Route to a cFS command output port (function code + payload)
        CFS_TELEMETRY   @< Route to a cFS telemetry output port (time + payload)
    }

    @ cFS system time, mirroring CFE_TIME_SysTime_t
    struct CfsTime {
        seconds: U32     @< Seconds since epoch
        subseconds: U32  @< Fractional seconds in 2^-32 second units
    }

    @ An entry in the CfsRouter APID routing table
    struct CfsRouteEntry {
        apid: ComCfg.Apid         @< The APID to route
        routeType: CfsRouteType   @< The category of output port to route to
        portIndex: FwIndexType    @< The index within that category's output port array
    }
}
