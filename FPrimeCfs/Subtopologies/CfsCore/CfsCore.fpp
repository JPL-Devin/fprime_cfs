module CfsCore {
    # ----------------------------------------------------------------------
    # Active Components
    # ----------------------------------------------------------------------
    instance cmdDisp: Svc.CommandDispatcher base id CfsCoreConfig.BASE_ID + 0x00000 \
        queue size CfsCoreConfig.QueueSizes.cmdDisp \
        stack size CfsCoreConfig.StackSizes.cmdDisp \
        priority CfsCoreConfig.Priorities.cmdDisp

    instance events: Svc.EventManager base id CfsCoreConfig.BASE_ID + 0x06000 \
        queue size CfsCoreConfig.QueueSizes.events \
        stack size CfsCoreConfig.StackSizes.events \
        priority CfsCoreConfig.Priorities.events

    # NOTE: Svc.TlmPacketizer requires a packet list to be set before use. The
    # packet list is generated from the deployment's telemetry packet
    # specification, so deployments set their own generated packet list during
    # component configuration. The default configuration below sets an empty
    # packet list so that a deployment that has not yet done so degrades
    # gracefully (no packets downlinked) instead of asserting on the first
    # telemetry write.
    instance tlmSend: Svc.TlmPacketizer base id CfsCoreConfig.BASE_ID + 0x01000 \
        queue size CfsCoreConfig.QueueSizes.tlmSend \
        stack size CfsCoreConfig.StackSizes.tlmSend \
        priority CfsCoreConfig.Priorities.tlmSend \
    {
        phase Fpp.ToCpp.Phases.configObjects """
        Svc::TlmPacketizerPacketList packetList = {{nullptr}, 0};
        Svc::TlmPacketizerPacket omittedChannels = {nullptr, 0, 0, 0};
        """

        phase Fpp.ToCpp.Phases.configComponents """
        CfsCore::tlmSend.setPacketList(
            ConfigObjects::CfsCore_tlmSend::packetList,
            ConfigObjects::CfsCore_tlmSend::omittedChannels,
            0
        );
        """
    }

    # ----------------------------------------------------------------------
    # Queued Components
    # ----------------------------------------------------------------------
    instance $health: Svc.Health base id CfsCoreConfig.BASE_ID + 0x02000 \
        queue size CfsCoreConfig.QueueSizes.$health \
    {
        phase Fpp.ToCpp.Phases.configConstants """
        enum {
            HEALTH_WATCHDOG_CODE = 0x123
        };
        """
        phase Fpp.ToCpp.Phases.configComponents """
        // Health is supplied a set of ping entries.
        CfsCore::health.setPingEntries(
            ConfigObjects::CfsCore_health::pingEntries,
            FW_NUM_ARRAY_ELEMENTS(ConfigObjects::CfsCore_health::pingEntries),
            ConfigConstants::CfsCore_health::HEALTH_WATCHDOG_CODE
        );
        """
    }

    # ----------------------------------------------------------------------
    # Passive Components
    # ----------------------------------------------------------------------
    instance version: Svc.Version base id CfsCoreConfig.BASE_ID + 0x03000 \
    {
        phase Fpp.ToCpp.Phases.configComponents """
        // Startup TLM and Config verbosity for Versions
        CfsCore::version.config(true);
        """
    }

    @ Text logger replacement mirroring F Prime events to cFS EVS
    instance evsMirror: FPrimeCfs.EvsMirror base id CfsCoreConfig.BASE_ID + 0x04000

    instance fatalAdapter: Svc.AssertFatalAdapter base id CfsCoreConfig.BASE_ID + 0x05000

    @ Rate group driver ticked by cFS scheduler (SCH) messages
    instance schAppDriver: FPrimeCfs.SchAppDriver base id CfsCoreConfig.BASE_ID + 0x08000

    @ Time component backed by cFS time services (CFE_TIME)
    instance cfsTime: FPrimeCfs.CfsSystemTime base id CfsCoreConfig.BASE_ID + 0x09000

    # This subtopology boxes the core command and data handling stack of an F Prime
    # cFS application, modeled after CdhCore.Subtopology with cFS-specific choices:
    # text logging is replaced by the FPrimeCfs.EvsMirror, telemetry uses the
    # Svc.TlmPacketizer, time is provided by the FPrimeCfs.CfsSystemTime component,
    # and the FPrimeCfs.SchAppDriver drives rate groups from cFS scheduler messages.
    #
    # Deployments using this subtopology declare the pattern connections:
    #
    #   command connections instance CfsCore.cmdDisp
    #   telemetry connections instance CfsCore.tlmSend
    #   text event connections instance CfsCore.evsMirror
    #   health connections instance CfsCore.$health
    #   time connections instance CfsCore.cfsTime
    #   event connections instance CfsCore.events
    topology Subtopology {
        # Active Components
        instance cmdDisp
        instance events
        instance tlmSend

        # Queued Components
        instance $health

        # Passive Components
        instance version
        instance evsMirror
        instance fatalAdapter
        instance fatalHandler
        instance schAppDriver
        instance cfsTime

        connections FaultProtection {
            events.FatalAnnounce -> fatalHandler.FatalReceive
        }

        # ----------------------------------------------------------------------
        # Topology ports
        # ----------------------------------------------------------------------

        @ Input port for receiving command buffers from routers, sequencers, or other command buffer sources
        port seqCmdBuff   = cmdDisp.seqCmdBuff

        @ Output port returning command execution status to the command source
        port seqCmdStatus = cmdDisp.seqCmdStatus

        @ Output port array sending packetized telemetry to the comm stack for downlink
        port tlmSendPktSend = tlmSend.PktSend

        @ Input port to trigger a telemetry packet send cycle
        port tlmSendRun = tlmSend.Run

        @ Input port for scheduling the command dispatcher
        port cmdDispRun = cmdDisp.run

        @ Input port for scheduling the Health component
        port healthRun  = $health.Run

        @ Input port receiving routed cFS scheduler command messages into the SchAppDriver
        port cfsCommandIn = schAppDriver.cfsCommandIn

        @ Output port returning ownership of buffers received on cfsCommandIn to their sender
        port schBufferReturnOut = schAppDriver.bufferReturnOut

        @ Output port producing one tick per scheduler message (connect to a Svc.RateGroupDriver)
        port cycleOut = schAppDriver.CycleOut

        @ Input port converting an F Prime time to a cFS system time
        port cfsTimeConvert = cfsTime.cfsTimeConvert

        @ Output port for sending event packets from the EventManager
        port eventsPktSend = events.PktSend

        @ Input port for scheduling the EventManager (dropped event telemetry)
        port eventsRun = events.run
    } # end Subtopology
} # end CfsCore
