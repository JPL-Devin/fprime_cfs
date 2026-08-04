module CfsCore {

    # The telemetry sender for CfsCore is the Svc.TlmPacketizer, which downlinks
    # telemetry as packets defined by a project-specific packet list.
    #
    # NOTE: Svc.TlmPacketizer requires a packet list to be set before use. The
    # packet list is generated from the deployment's telemetry packet
    # specification, so projects must override this configuration file
    # (register_fprime_config CONFIGURATION_OVERRIDES) to supply the
    # setPacketList call with their own generated packet list, e.g.:
    #
    # instance tlmSend: Svc.TlmPacketizer base id CfsCoreConfig.BASE_ID + 0x01000 \
    #     queue size CfsCoreConfig.QueueSizes.tlmSend \
    #     stack size CfsCoreConfig.StackSizes.tlmSend \
    #     priority CfsCoreConfig.Priorities.tlmSend \
    # {
    #     phase Fpp.ToCpp.Phases.configComponents """
    #     CfsCore::tlmSend.setPacketList(
    #         MyDeployment::MyDeployment_MyPacketsTlmPackets::packetList,
    #         MyDeployment::MyDeployment_MyPacketsTlmPackets::omittedChannels,
    #         1
    #     );
    #     """
    # }
    # The default (unoverridden) instance sets an empty packet list so that a
    # deployment missing the override degrades gracefully (no packets downlinked)
    # instead of asserting on the first telemetry write.
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
}
