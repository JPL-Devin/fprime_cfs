#####
# FPrimeCfs types:
#
# Common type definitions shared across FPrimeCfs components
#####

module FPrimeCfs {

    @ cFS system time, mirroring CFE_TIME_SysTime_t
    struct CfsTime {
        seconds: U32     @< Seconds since epoch
        subseconds: U32  @< Fractional seconds in 2^-32 second units
    }
}
