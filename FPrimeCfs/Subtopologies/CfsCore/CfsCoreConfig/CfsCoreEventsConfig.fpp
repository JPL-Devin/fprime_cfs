module CfsCore {

    # The optional Svc.EventManager, instantiated only by the
    # CfsCore.SubtopologyWithEvents topology. Deployments importing
    # CfsCore.Subtopology do not include this instance.
    instance events: Svc.EventManager base id CfsCoreConfig.BASE_ID + 0x06000 \
        queue size CfsCoreConfig.QueueSizes.events \
        stack size CfsCoreConfig.StackSizes.events \
        priority CfsCoreConfig.Priorities.events
}
