#####
# CfsCmdRouter types:
#
# Types for the CfsCmdRouter
#####

module FPrimeCfs {

    @ The category of output port that a function code routes to
    enum CfsCmdRouteType : U8 {
        COM     @< Route to a com output port (Fw.Com, copied without the secondary header)
        BUFFER  @< Route to a buffer output port (function code + payload)
    }

    @ An entry in the CfsCmdRouter function code routing table
    struct CfsCmdRouteEntry {
        functionCode: U8              @< The cFS command function code to route
        routeType: CfsCmdRouteType    @< The category of output port to route to
        portIndex: FwIndexType        @< The index within that category's output port array
    }
}
