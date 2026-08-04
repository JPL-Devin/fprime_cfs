module FPrimeCfs {

    @ Framer stage that wraps outgoing telemetry in a cFS telemetry secondary
    @ header (big-endian 4-byte seconds, 2-byte subseconds) ahead of a downstream
    @ space packet framer (e.g. Svc.Ccsds.SpacePacketFramer). The time is
    @ extracted from the incoming F Prime packet (event, telemetry channel, or
    @ packetized telemetry) and converted to cFS system time via the
    @ cfsTimeConvert port. Sets the secondary header flag in the frame context
    @ so the downstream framer marks the packet accordingly.
    passive component CfsTlmFramer {

        import Svc.Framer

        @ Port to allocate a buffer for the wrapped packet
        output port bufferAllocate: Fw.BufferGet

        @ Port to deallocate a buffer once the wrapped packet is returned
        output port bufferDeallocate: Fw.BufferSend

        @ Port to convert the extracted F Prime time to a cFS system time.
        @ When unconnected, the conversion is computed directly from the
        @ F Prime seconds and microseconds.
        output port cfsTimeConvert: CfsTimeConvert

        # ----------------------------------------------------------------------
        # Events
        # ----------------------------------------------------------------------

        @ The allocator did not return a buffer large enough for the wrapped packet;
        @ the packet was dropped
        event AllocationFailed(
                $size: U32 @< The requested allocation size in bytes
            ) \
            severity warning high \
            format "Failed to allocate a {} byte buffer for a cFS telemetry packet"

        @ A time tag could not be extracted from an incoming packet; the current
        @ system time was used instead
        event TimeExtractionFailed(
                descriptor: U16 @< The packet descriptor of the incoming packet
            ) \
            severity warning low \
            format "Could not extract a time tag from packet with descriptor {}; using current time"

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
