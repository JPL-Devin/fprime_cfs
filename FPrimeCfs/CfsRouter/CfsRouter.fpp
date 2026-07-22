module FPrimeCfs {

    @ Routes packets deframed by an Svc.Ccsds.SpacePacketDeframer to the rest of the system.
    @ Replaces Svc.FprimeRouter in a cFS-hosted topology: F Prime commands are copied and
    @ forwarded like Svc.FprimeRouter; cFS commands and telemetry are forwarded on custom
    @ ports carrying the secondary-header fields (function code, or time) and the payload;
    @ anything without a configured route goes to the unknown output.
    @
    @ Routing is selected by the static APID -> (route type, port index) table configured
    @ in CfsRouterCfg.fpp (CFS_ROUTER_ROUTE_TABLE).
    passive component CfsRouter {

        @ The APID routing table type, statically configured via CfsRouterCfg.fpp
        array CfsRouteTable = [CFS_ROUTER_ROUTE_TABLE_SIZE] CfsRouteEntry default CFS_ROUTER_ROUTE_TABLE

        # ----------------------------------------------------------------------
        # Router <-> Deframer
        # ----------------------------------------------------------------------

        @ Receiving data (Fw::Buffer) to be routed, with the deframer-provided context
        sync input port dataIn: Svc.ComDataWithContext

        @ Port for returning ownership of data received on dataIn, with the context
        @ it was received with
        output port dataReturnOut: Svc.ComDataWithContext

        # ----------------------------------------------------------------------
        # F Prime command route
        # ----------------------------------------------------------------------

        @ Port for sending F Prime command packets as Fw::ComBuffers (copied)
        output port commandOut: [CFS_ROUTER_FPRIME_COMMAND_PORTS] Fw.Com

        @ Port for receiving command responses from a command dispatcher (no-op)
        sync input port cmdResponseIn: Fw.CmdResponse

        # ----------------------------------------------------------------------
        # cFS routes (ownership transferred; returned on bufferReturnIn)
        # ----------------------------------------------------------------------

        @ Port for sending cFS commands (function code + payload)
        output port cfsCommandOut: [CFS_ROUTER_CFS_COMMAND_PORTS] FPrimeCfs.CfsCommand

        @ Port for sending cFS telemetry (time + payload)
        output port cfsTelemetryOut: [CFS_ROUTER_CFS_TELEMETRY_PORTS] FPrimeCfs.CfsTelemetry

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

        @ Too many buffers are outstanding on the pass-through routes; the packet was
        @ returned to the sender unrouted
        event TooManyPendingBuffers(
                apid: U16   @< The APID of the packet
            ) \
            severity warning high \
            format "Packet with APID {} dropped: too many pending buffers"

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
