module FPrimeCfs {
    @ Framer inserting the cFS telemetry secondary header into downlinked space packets.
    @
    @ Data provided on `dataIn` is one or more complete CCSDS space packets (e.g. from an
    @ Svc.Ccsds.SpacePacketFramer, possibly concatenated by an Svc.ComAggregator). Each telemetry
    @ packet without a secondary header is rewritten as a valid cFS telemetry packet: the secondary
    @ header flag is set in the primary header, the length field is adjusted, and the 6-byte cFS
    @ telemetry secondary header (big-endian 4-byte seconds, 2-byte subseconds) is inserted between
    @ the primary header and the packet payload. Command packets and packets already carrying a
    @ secondary header pass through unmodified.
    passive component CfsTlmFramer {

        import Svc.Framer

        @ Port to allocate a buffer for the framed packets
        output port bufferAllocate: Fw.BufferGet

        @ Port to deallocate a framed buffer once returned from downstream
        output port bufferDeallocate: Fw.BufferSend

        @ Port for requesting the current time used to fill the telemetry secondary header
        time get port timeCaller

    }
}
