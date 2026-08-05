module FPrimeCfs {

    @ Routes cFS command messages to consumers based on the function code in the cFS
    @ command secondary header. Instantiated after a CfsRouter (which routes APIDs to
    @ consumers): the incoming data carries the cFS command secondary header
    @ ({U8 FunctionCode, U8 Checksum}) ahead of the command payload. Messages failing
    @ the cFS command checksum are dropped with a warning; anything without a
    @ configured route goes to the unknown output.
    @
    @ Routing is selected by the static function code -> (route type, port index)
    @ table configured in CfsCmdRouterCfg.fpp (CFS_CMD_ROUTER_ROUTE_TABLE).
    passive component CfsCmdRouter {

        @ The function code routing table type, statically configured via CfsCmdRouterCfg.fpp
        array CfsCmdRouteTable = [CFS_CMD_ROUTER_ROUTE_TABLE_SIZE] CfsCmdRouteEntry default CFS_CMD_ROUTER_ROUTE_TABLE

        # ----------------------------------------------------------------------
        # CmdRouter <-> Router
        # ----------------------------------------------------------------------

        @ Receiving data (Fw::Buffer) to be routed, with the context it was
        @ deframed with. The data starts at the cFS command secondary header.
        @ Sync (not guarded): outputs may be invoked from downstream return paths on
        @ the same call stack; the pending-buffer store is protected by an internal mutex.
        sync input port dataIn: Svc.ComDataWithContext

        @ Port for returning ownership of data received on dataIn, with the context
        @ it was received with
        output port dataReturnOut: Svc.ComDataWithContext

        # ----------------------------------------------------------------------
        # Com route (copied)
        # ----------------------------------------------------------------------

        @ Port for sending the message without the secondary header as an
        @ Fw::ComBuffer (copied), e.g. to an F Prime command dispatcher
        output port comOut: [CFS_CMD_ROUTER_COM_PORTS] Fw.Com

        @ Port for receiving command responses from a command dispatcher (no-op)
        sync input port cmdResponseIn: Fw.CmdResponse

        # ----------------------------------------------------------------------
        # Buffer route (ownership transferred; returned on bufferReturnIn)
        # ----------------------------------------------------------------------

        @ Port for sending the payload (Fw::Buffer without the secondary header)
        @ with the function code as a separate argument
        output port bufferOut: [CFS_CMD_ROUTER_BUFFER_PORTS] FPrimeCfs.CfsCommand

        @ Port for forwarding messages with no configured route.
        @ Ownership of the buffer is passed to the receiver, which must return it
        @ via bufferReturnIn when done.
        output port unknownDataOut: Svc.ComDataWithContext

        @ Port for receiving back ownership of buffers sent on bufferOut or
        @ unknownDataOut.
        @ Sync (not guarded): may be invoked synchronously from consumers of bufferOut or
        @ unknownDataOut while dataIn is on the stack; the pending-buffer store is
        @ protected by an internal mutex.
        sync input port bufferReturnIn: Fw.BufferSend

        # ----------------------------------------------------------------------
        # Events
        # ----------------------------------------------------------------------

        @ A message failed the cFS command checksum and was dropped
        event BadChecksum(
                apid: U16           @< The APID of the message
                functionCode: U8    @< The function code of the message
                checksum: U8        @< The XOR residual over the message (zero when valid)
            ) \
            severity warning high \
            format "Command with APID {} function code {} dropped: bad checksum (residual {})"

        @ A message is missing its cFS command secondary header
        event MissingSecondaryHeader(
                apid: U16       @< The APID of the message
                dataSize: U32   @< The size of the message data in bytes
            ) \
            severity warning high \
            format "Command with APID {} (size {}) is missing the required cFS command secondary header"

        @ An error occurred while copying a message into a com buffer
        event SerializationError(
                status: U32 @< The status of the operation
            ) \
            severity warning high \
            format "Serializing com buffer failed with status {}"

        @ Too many buffers are outstanding on the pass-through routes; the message was
        @ returned to the sender unrouted
        event TooManyPendingBuffers(
                apid: U16   @< The APID of the message
            ) \
            severity warning high \
            format "Command with APID {} dropped: too many pending buffers"

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
