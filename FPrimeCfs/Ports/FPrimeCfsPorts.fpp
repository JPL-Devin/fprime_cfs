#####
# FPrimeCfs ports:
#
# Common port definitions for carrying cFS messages within F Prime
#####

module FPrimeCfs {

    @ A cFS command: function code from the command secondary header plus the payload.
    @ Ownership of the buffer is passed to the receiver, which must return it
    @ via the sender's buffer return port when done.
    port CfsCommand(
        functionCode: U8    @< Function code from the cFS command secondary header
        ref data: Fw.Buffer @< Command payload (data after the secondary header)
    )

    @ A cFS telemetry message: time from the telemetry secondary header plus the payload.
    @ Ownership of the buffer is passed to the receiver, which must return it
    @ via the sender's buffer return port when done.
    port CfsTelemetry(
        sysTime: CfsTime    @< Time from the cFS telemetry secondary header
        ref data: Fw.Buffer @< Telemetry payload (data after the secondary header)
    )

    @ Converts an F Prime time to a cFS system time (CFE_TIME_SysTime_t mirror)
    port CfsTimeConvert(
        $time: Fw.Time @< The F Prime time to convert
    ) -> CfsTime
}
