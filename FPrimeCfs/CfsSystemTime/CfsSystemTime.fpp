module FPrimeCfs {
    @ Time component backed by cFS time services (CFE_TIME). Implements the
    @ standard time interface by constructing Fw.Time objects from the seconds
    @ and subseconds returned by CFE_TIME_GetTime, and provides a conversion
    @ port for translating an Fw.Time back into a cFS system time.
    passive component CfsSystemTime {

        @ Implement the time interface
        import Svc.Time

        @ Port converting an F Prime time back to a cFS system time
        @ (CFE_TIME_SysTime_t mirror)
        sync input port cfsTimeConvert: CfsTimeConvert

    }
}
