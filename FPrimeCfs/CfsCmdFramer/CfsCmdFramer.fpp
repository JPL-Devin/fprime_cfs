module FPrimeCfs {

    @ Framer stage that wraps outgoing data in a cFS command secondary header
    @ ({U8 FunctionCode, U8 Checksum}) ahead of a downstream space packet framer
    @ (e.g. Svc.Ccsds.SpacePacketFramer). Sets the secondary header flag in the
    @ frame context so the downstream framer marks the packet accordingly.
    passive component CfsCmdFramer {

        import Svc.Framer

        @ Port to allocate a buffer for the wrapped packet
        output port bufferAllocate: Fw.BufferGet

        @ Port to deallocate a buffer once the wrapped packet is returned
        output port bufferDeallocate: Fw.BufferSend

        # ----------------------------------------------------------------------
        # Events
        # ----------------------------------------------------------------------

        @ The allocator did not return a buffer large enough for the wrapped packet;
        @ the packet was dropped
        event AllocationFailed(
                $size: U32 @< The requested allocation size in bytes
            ) \
            severity warning high \
            format "Failed to allocate a {} byte buffer for a cFS command packet"

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
