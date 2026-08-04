module FPrimeCfs {
    @ Deframer removing the cFS telemetry secondary header from received space packets.
    @
    @ Data provided on `dataIn` is a single complete CCSDS space packet (e.g. a cFS message received
    @ from the software bus by a CfsBridge). A telemetry packet carrying a secondary header is
    @ rewritten in place as a bare F Prime telemetry space packet: the 6-byte cFS telemetry
    @ secondary header is removed, the secondary header flag is cleared in the primary header, and
    @ the length field is adjusted. Command packets and telemetry packets without a secondary header
    @ pass through unmodified.
    passive component CfsTlmDeframer {

        import Svc.Deframer

    }
}
