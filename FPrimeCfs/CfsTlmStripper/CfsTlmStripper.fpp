module FPrimeCfs {

    @ Framer stage that strips the cFS telemetry secondary header (big-endian
    @ 4-byte seconds, 2-byte subseconds) from complete telemetry space packets,
    @ rebuilding the CCSDS primary header (length field and secondary header
    @ flag). The inverse of FPrimeCfs.CfsTlmFramer, for ground-facing bridges
    @ (e.g. a GDS bridge) whose downstream consumers expect bare F Prime
    @ packets inside the space packets. The strip is performed in place: the
    @ rebuilt primary header is written over the trailing bytes of the
    @ secondary header and the buffer's data pointer is advanced, so no
    @ allocation or copy is made. Command packets and packets without the
    @ secondary header flag are forwarded unchanged.
    passive component CfsTlmStripper {

        import Svc.Framer

        # ----------------------------------------------------------------------
        # Events
        # ----------------------------------------------------------------------

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
