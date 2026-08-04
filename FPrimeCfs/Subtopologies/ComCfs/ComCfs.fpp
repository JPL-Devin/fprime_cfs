module ComCfs {

    # ----------------------------------------------------------------------
    # Active Components
    # ----------------------------------------------------------------------
    instance aggregator: Svc.ComAggregator base id ComCfsConfig.BASE_ID + 0x00000 \
        queue size ComCfsConfig.QueueSizes.aggregator \
        stack size ComCfsConfig.StackSizes.aggregator \
        priority ComCfsConfig.Priorities.aggregator

    # ----------------------------------------------------------------------
    # Queued Components
    # ----------------------------------------------------------------------
    instance cfsBridge: FPrimeCfs.CfsBridge base id ComCfsConfig.BASE_ID + 0x01000 \
        queue size ComCfsConfig.QueueSizes.cfsBridge

    # ----------------------------------------------------------------------
    # Passive Components
    # ----------------------------------------------------------------------
    instance commsBufferManager: Svc.BufferManager base id ComCfsConfig.BASE_ID + 0x02000 \
    {
        phase Fpp.ToCpp.Phases.configObjects """
        Svc::BufferManager::BufferBins bins;
        """

        phase Fpp.ToCpp.Phases.configComponents """
        memset(&ConfigObjects::ComCfs_commsBufferManager::bins, 0, sizeof(ConfigObjects::ComCfs_commsBufferManager::bins));
        ConfigObjects::ComCfs_commsBufferManager::bins.bins[0].bufferSize = ComCfsConfig::BuffMgr::commsBuffSize;
        ConfigObjects::ComCfs_commsBufferManager::bins.bins[0].numBuffers = ComCfsConfig::BuffMgr::commsBuffCount;
        ConfigObjects::ComCfs_commsBufferManager::bins.bins[1].bufferSize = ComCfsConfig::BuffMgr::commsFileBuffSize;
        ConfigObjects::ComCfs_commsBufferManager::bins.bins[1].numBuffers = ComCfsConfig::BuffMgr::commsFileBuffCount;
        ComCfs::commsBufferManager.setup(
            ComCfsConfig::BuffMgr::commsBuffMgrId,
            0,
            ComCfs::Allocation::memAllocator,
            ConfigObjects::ComCfs_commsBufferManager::bins
        );
        """

        phase Fpp.ToCpp.Phases.tearDownComponents """
        ComCfs::commsBufferManager.cleanup();
        """
    }

    @ App bridge for outgoing cFS commands, feeding the cFS command framer
    instance cmdBridge: FPrimeCfs.CfsAppBridge base id ComCfsConfig.BASE_ID + 0x03000 \
    {
        phase Fpp.ToCpp.Phases.configComponents """
        ComCfs::cmdBridge.configure(ComCfs::BridgeConfig::commandApid);
        """
    }

    @ App bridge for outgoing telemetry and events, feeding the cFS telemetry framer
    instance tlmBridge: FPrimeCfs.CfsAppBridge base id ComCfsConfig.BASE_ID + 0x04000

    instance cmdFramer: FPrimeCfs.CfsCmdFramer base id ComCfsConfig.BASE_ID + 0x05000

    instance tlmFramer: FPrimeCfs.CfsTlmFramer base id ComCfsConfig.BASE_ID + 0x06000

    instance spacePacketFramer: Svc.Ccsds.SpacePacketFramer base id ComCfsConfig.BASE_ID + 0x07000

    instance spacePacketDeframer: Svc.Ccsds.SpacePacketDeframer base id ComCfsConfig.BASE_ID + 0x08000

    instance apidManager: Svc.Ccsds.ApidManager base id ComCfsConfig.BASE_ID + 0x09000

    instance cfsRouter: FPrimeCfs.CfsRouter base id ComCfsConfig.BASE_ID + 0x0A000

    instance cfsCmdRouter: FPrimeCfs.CfsCmdRouter base id ComCfsConfig.BASE_ID + 0x0B000

    # This subtopology boxes the full cFS communications stack of an F Prime cFS
    # application, modeled after ComCcsds.SpacePacketFraming:
    #
    # - Downlink: two app bridges (commands and telemetry) feed the cFS secondary
    #   framers (FPrimeCfs.CfsCmdFramer and FPrimeCfs.CfsTlmFramer), which feed the
    #   space packet framer, the aggregator, and finally the CfsBridge onto the cFS
    #   software bus.
    # - Uplink: the CfsBridge feeds the space packet deframer, followed by the
    #   FPrimeCfs.CfsRouter (APID routing) and the FPrimeCfs.CfsCmdRouter (function
    #   code routing) ahead of commanding.
    topology Subtopology {
        # Active Components
        instance aggregator

        # Queued Components
        instance cfsBridge

        # Passive Components
        instance commsBufferManager
        instance cmdBridge
        instance tlmBridge
        instance cmdFramer
        instance tlmFramer
        instance spacePacketFramer
        instance spacePacketDeframer
        instance apidManager
        instance cfsRouter
        instance cfsCmdRouter

        connections Downlink {
            # Command chain: app bridge -> cFS command framer
            cmdBridge.dataOut        -> cmdFramer.dataIn
            cmdFramer.dataReturnOut  -> cmdBridge.dataReturnIn
            cmdBridge.bufferAllocate   -> commsBufferManager.bufferGetCallee
            cmdBridge.bufferDeallocate -> commsBufferManager.bufferSendIn
            cmdFramer.bufferAllocate   -> commsBufferManager.bufferGetCallee
            cmdFramer.bufferDeallocate -> commsBufferManager.bufferSendIn

            # Telemetry chain: app bridge -> cFS telemetry framer
            tlmBridge.dataOut        -> tlmFramer.dataIn
            tlmFramer.dataReturnOut  -> tlmBridge.dataReturnIn
            tlmBridge.bufferAllocate   -> commsBufferManager.bufferGetCallee
            tlmBridge.bufferDeallocate -> commsBufferManager.bufferSendIn
            tlmFramer.bufferAllocate   -> commsBufferManager.bufferGetCallee
            tlmFramer.bufferDeallocate -> commsBufferManager.bufferSendIn

            # Both secondary framers feed the space packet framer (fan-in). The
            # wrapped buffers coming back on spacePacketFramer.dataReturnOut are
            # deallocated by cmdFramer into the shared commsBufferManager pool;
            # both secondary framers allocate from that same pool, so the return
            # path does not need to discriminate by chain.
            cmdFramer.dataOut -> spacePacketFramer.dataIn
            tlmFramer.dataOut -> spacePacketFramer.dataIn
            spacePacketFramer.dataReturnOut -> cmdFramer.dataReturnIn

            # SpacePacketFramer buffer and APID management
            spacePacketFramer.bufferAllocate   -> commsBufferManager.bufferGetCallee
            spacePacketFramer.bufferDeallocate -> commsBufferManager.bufferSendIn
            spacePacketFramer.getApidSeqCount  -> apidManager.getApidSeqCountIn

            # SpacePacketFramer <-> ComAggregator
            spacePacketFramer.dataOut -> aggregator.dataIn
            aggregator.dataReturnOut  -> spacePacketFramer.dataReturnIn

            # ComAggregator <-> CfsBridge (bottom of the stack, onto the cFS software bus)
            aggregator.dataOut      -> cfsBridge.dataIn
            cfsBridge.dataReturnOut -> aggregator.dataReturnIn

            # ComStatus. The status chain terminates at the space packet framer:
            # the app bridges do not pace transmission on communication status.
            cfsBridge.comStatusOut  -> aggregator.comStatusIn
            aggregator.comStatusOut -> spacePacketFramer.comStatusIn
        }

        connections Uplink {
            # CfsBridge <-> SpacePacketDeframer
            cfsBridge.dataOut               -> spacePacketDeframer.dataIn
            spacePacketDeframer.dataReturnOut -> cfsBridge.dataReturnIn
            # SpacePacketDeframer APID validation
            spacePacketDeframer.validateApidSeqCount -> apidManager.validateApidSeqCountIn

            # SpacePacketDeframer <-> CfsRouter (APID routing)
            spacePacketDeframer.dataOut -> cfsRouter.dataIn
            cfsRouter.dataReturnOut     -> spacePacketDeframer.dataReturnIn

            # CfsRouter <-> CfsCmdRouter (function code routing before commanding).
            # APIDs without a configured CfsRouter route (including this app's own
            # command message ID by default) flow to the command router.
            cfsRouter.unknownDataOut    -> cfsCmdRouter.dataIn
            cfsCmdRouter.dataReturnOut  -> cfsRouter.dataReturnIn
        }

        # ----------------------------------------------------------------------
        # Topology ports (app bridge boundary - downlink inputs)
        # ----------------------------------------------------------------------

        @ Input port receiving outgoing cFS commands (function code + payload) into the command app bridge
        port cfsCommandIn = cmdBridge.cfsCommandIn

        @ Output port returning ownership of buffers received on cfsCommandIn to their sender
        port cfsCommandBufferReturnOut = cmdBridge.bufferReturnOut

        @ Input port array receiving outgoing F Prime com buffers (telemetry, events,
        @ packetized telemetry) into the telemetry app bridge
        port comIn = tlmBridge.comIn

        @ Output port converting extracted F Prime times to cFS system times
        @ (connect to a FPrimeCfs.CfsSystemTime cfsTimeConvert input)
        port cfsTimeConvertOut = tlmFramer.cfsTimeConvert

        # ----------------------------------------------------------------------
        # Topology ports (routing boundary - uplink outputs)
        # ----------------------------------------------------------------------

        @ Output port array sending routed F Prime command packets to the command dispatcher
        port commandOut = cfsRouter.commandOut

        @ Input port receiving command response messages back into the APID router
        port cmdResponseIn = cfsRouter.cmdResponseIn

        @ Output port array sending routed cFS commands (function code + payload)
        port cfsCommandOut = cfsRouter.cfsCommandOut

        @ Output port array sending routed cFS telemetry (time + payload)
        port cfsTelemetryOut = cfsRouter.cfsTelemetryOut

        @ Input port receiving back ownership of buffers sent on cfsCommandOut or cfsTelemetryOut
        port bufferReturnIn = cfsRouter.bufferReturnIn

        @ Output port sending uplinked file packets to the file handling stack
        port fileUplinkOut = cfsRouter.fileOut

        @ Input port receiving back buffer ownership from the file handling stack
        port fileUplinkReturnIn = cfsRouter.fileBufferReturnIn

        @ Output port array sending function-code-routed messages as Fw::ComBuffers
        @ (e.g. to an F Prime command dispatcher)
        port cmdRouterComOut = cfsCmdRouter.comOut

        @ Input port receiving command response messages back into the command router
        port cmdRouterCmdResponseIn = cfsCmdRouter.cmdResponseIn

        @ Output port array sending function-code-routed payloads (function code + payload)
        port cmdRouterBufferOut = cfsCmdRouter.bufferOut

        @ Output port forwarding messages with no configured function code route
        port cmdRouterUnknownDataOut = cfsCmdRouter.unknownDataOut

        @ Input port receiving back ownership of buffers sent on cmdRouterBufferOut or cmdRouterUnknownDataOut
        port cmdRouterBufferReturnIn = cfsCmdRouter.bufferReturnIn

        # ----------------------------------------------------------------------
        # Topology ports (scheduling)
        # ----------------------------------------------------------------------

        @ Rate-group driven timeout to flush the ComAggregator buffer
        port aggregatorTimeout = aggregator.timeout

        @ Input port triggering commsBufferManager telemetry output
        port bufferManagerSchedIn = commsBufferManager.schedIn
    } # end Subtopology
} # end ComCfs
