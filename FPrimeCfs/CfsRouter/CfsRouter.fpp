module FPrimeCfs {

    @ Number of F Prime command output ports on the CfsRouter
    constant CfsRouterFprimeCommandPorts = 10

    @ Number of cFS command output ports on the CfsRouter
    constant CfsRouterCfsCommandPorts = 10

    @ Number of cFS telemetry output ports on the CfsRouter
    constant CfsRouterCfsTelemetryPorts = 10

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

    @ Routes packets deframed by an Svc.Ccsds.SpacePacketDeframer to the rest of the system.
    @ Replaces Svc.FprimeRouter in a cFS-hosted topology: F Prime commands are copied and
    @ forwarded like Svc.FprimeRouter; cFS commands and telemetry are forwarded on custom
    @ ports carrying the secondary-header fields (function code, or time) and the payload;
    @ anything without a configured route goes to the unknown output.
    @
    @ Routing is selected by a configurable APID -> (route type, port index) table supplied
    @ at topology configuration time via configure().
    passive component CfsRouter {

        # ----------------------------------------------------------------------
        # Router <-> Deframer
        # ----------------------------------------------------------------------

        @ Receiving data (Fw::Buffer) to be routed, with the deframer-provided context
        sync input port dataIn: Svc.ComDataWithContext

        @ Port for returning ownership of data received on dataIn
        output port dataReturnOut: Svc.ComDataWithContext

        # ----------------------------------------------------------------------
        # F Prime command route
        # ----------------------------------------------------------------------

        @ Port for sending F Prime command packets as Fw::ComBuffers (copied)
        output port commandOut: [CfsRouterFprimeCommandPorts] Fw.Com

        @ Port for receiving command responses from a command dispatcher (no-op)
        sync input port cmdResponseIn: Fw.CmdResponse

        # ----------------------------------------------------------------------
        # cFS routes (ownership transferred; returned on bufferReturnIn)
        # ----------------------------------------------------------------------

        @ Port for sending cFS commands (function code + payload)
        output port cfsCommandOut: [CfsRouterCfsCommandPorts] FPrimeCfs.CfsCommand

        @ Port for sending cFS telemetry (time + payload)
        output port cfsTelemetryOut: [CfsRouterCfsTelemetryPorts] FPrimeCfs.CfsTelemetry

        @ Port for forwarding packets with no configured route.
        @ Ownership of the buffer is passed to the receiver, which must return it
        @ via bufferReturnIn when done.
        output port unknownDataOut: Svc.ComDataWithContext

        @ Port for receiving back ownership of buffers sent on cfsCommandOut,
        @ cfsTelemetryOut, or unknownDataOut
        sync input port bufferReturnIn: Fw.BufferSend

        # ----------------------------------------------------------------------
        # Events
        # ----------------------------------------------------------------------

        @ An error occurred while copying a command packet into a com buffer
        event SerializationError(
                status: U32 @< The status of the operation
            ) \
            severity warning high \
            format "Serializing com buffer failed with status {}"

        @ A packet routed as a cFS command or telemetry message is missing its secondary header
        event MissingSecondaryHeader(
                apid: U16       @< The APID of the packet
                dataSize: U32   @< The size of the packet data in bytes
            ) \
            severity warning high \
            format "Packet with APID {} (size {}) is missing the required cFS secondary header"

        ###############################################################################
        # Standard AC Ports for Events
        ###############################################################################

        @ Port for requesting the current time
        time get port timeCaller

        @ Port for sending textual representation of events
        text event port logTextOut

        @ Port for sending events to downlink
        event port logOut

    }
}
