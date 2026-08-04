module CfsTlmFraming {

    # ----------------------------------------------------------------------
    # Passive Components
    # ----------------------------------------------------------------------

    instance cfsTlmFramer: FPrimeCfs.CfsTlmFramer base id CfsTlmFramingConfig.BASE_ID + 0x00000

    instance cfsTlmDeframer: FPrimeCfs.CfsTlmDeframer base id CfsTlmFramingConfig.BASE_ID + 0x01000

    # This subtopology boxes the downlink cFS telemetry framing layer: it inserts the cFS telemetry
    # secondary header into telemetry space packets so they traverse the cFS software bus as valid
    # cFS telemetry packets.
    topology Framing {
        # Usage Note:
        #
        # When importing this subtopology, users shall establish the following external connections:
        #
        # 1) Upstream (source of complete space packets, e.g. ComCcsds.SpacePacketFraming):
        #     - [upstream].dataOut                     -> CfsTlmFraming.Framing.dataIn
        #     - CfsTlmFraming.Framing.dataReturnOut    -> [upstream].dataReturnIn
        #     - CfsTlmFraming.Framing.comStatusOut     -> [upstream].comStatusIn
        # 2) Downstream (consumer of cFS telemetry packets, e.g. a FPrimeCfs.CfsBridge):
        #     - CfsTlmFraming.Framing.dataOut          -> [downstream].dataIn
        #     - [downstream].dataReturnOut             -> CfsTlmFraming.Framing.dataReturnIn
        #     - [downstream].comStatusOut              -> CfsTlmFraming.Framing.comStatusIn
        # 3) Buffer management (e.g. a Svc.BufferManager):
        #     - CfsTlmFraming.Framing.bufferAllocate   -> [BufferManager].bufferGetCallee
        #     - CfsTlmFraming.Framing.bufferDeallocate -> [BufferManager].bufferSendIn

        instance cfsTlmFramer

        # ----------------------------------------------------------------------
        # Topology ports
        # ----------------------------------------------------------------------

        @ Input port receiving complete space packets from the upstream packet layer
        port dataIn        = cfsTlmFramer.dataIn

        @ Output port returning ownership of upstream buffers once framing is handled
        port dataReturnOut = cfsTlmFramer.dataReturnOut

        @ Output port forwarding com status to the upstream packet layer
        port comStatusOut  = cfsTlmFramer.comStatusOut

        @ Output port sending cFS telemetry packets to the downstream consumer
        port dataOut       = cfsTlmFramer.dataOut

        @ Input port receiving back ownership of framed buffers from the downstream consumer
        port dataReturnIn  = cfsTlmFramer.dataReturnIn

        @ Input port receiving com status from the downstream consumer
        port comStatusIn   = cfsTlmFramer.comStatusIn

        # Buffer management boundary
        @ Output port for allocating framed packet buffers
        port bufferAllocate   = cfsTlmFramer.bufferAllocate

        @ Output port for deallocating framed packet buffers
        port bufferDeallocate = cfsTlmFramer.bufferDeallocate
    } # end Framing

    # This subtopology boxes the receive-side cFS telemetry deframing layer: it removes the cFS
    # telemetry secondary header from received telemetry space packets so downstream F Prime
    # framing layers see bare F Prime telemetry packets.
    topology Deframing {
        # Usage Note:
        #
        # When importing this subtopology, users shall establish the following external connections:
        #
        # 1) Upstream (source of received cFS messages, e.g. a FPrimeCfs.CfsBridge):
        #     - [upstream].dataOut                      -> CfsTlmFraming.Deframing.dataIn
        #     - CfsTlmFraming.Deframing.dataReturnOut   -> [upstream].dataReturnIn
        # 2) Downstream (consumer of bare F Prime space packets, e.g. a TM framer):
        #     - CfsTlmFraming.Deframing.dataOut         -> [downstream].dataIn
        #     - [downstream].dataReturnOut              -> CfsTlmFraming.Deframing.dataReturnIn

        instance cfsTlmDeframer

        # ----------------------------------------------------------------------
        # Topology ports
        # ----------------------------------------------------------------------

        @ Input port receiving complete space packets from the upstream source
        port dataIn        = cfsTlmDeframer.dataIn

        @ Output port returning ownership of upstream buffers once deframing is handled
        port dataReturnOut = cfsTlmDeframer.dataReturnOut

        @ Output port sending bare F Prime space packets to the downstream consumer
        port dataOut       = cfsTlmDeframer.dataOut

        @ Input port receiving back ownership of deframed buffers from the downstream consumer
        port dataReturnIn  = cfsTlmDeframer.dataReturnIn
    } # end Deframing

} # end CfsTlmFraming
