module FPrimeCfs {

    @ Framer stage that strips the cFS telemetry secondary header (big-endian
    @ 4-byte seconds, 2-byte subseconds) from complete telemetry space packets,
    @ rebuilding the CCSDS primary header (length field and secondary header
    @ flag). The inverse of FPrimeCfs.CfsTlmFramer, for ground-facing bridges
    @ (e.g. a GDS bridge) whose downstream consumers expect bare F Prime
    @ packets inside the space packets. Command packets and packets without
    @ the secondary header flag are forwarded unchanged (as a copy).
    passive component CfsTlmStripper {

        import Svc.Framer

        @ Port to allocate a buffer for the stripped packet
        output port bufferAllocate: Fw.BufferGet

        @ Port to deallocate a buffer once the stripped packet is returned
        output port bufferDeallocate: Fw.BufferSend

        # ----------------------------------------------------------------------
        # Events
        # ----------------------------------------------------------------------

        @ The allocator did not return a buffer large enough for the stripped packet;
        @ the packet was dropped
        event AllocationFailed(
                $size: U32 @< The requested allocation size in bytes
            ) \
            severity warning high \
            format "Failed to allocate a {} byte buffer for a stripped telemetry packet"

        @ An incoming buffer was not a complete space packet; the packet was dropped
        event MalformedPacket(
                $size: U32 @< The size of the incoming buffer in bytes
            ) \
            severity warning high \
            format "Dropped a {} byte buffer that does not hold a complete space packet"

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
