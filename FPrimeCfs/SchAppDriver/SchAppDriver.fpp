module FPrimeCfs {
    @ Rate group driver triggered by cFS scheduler (SCH) messages. Each scheduler
    @ message routed to this component from the cFS software bus (via the
    @ CfsBridge/router path) produces one tick on CycleOut, driving the
    @ Svc.RateGroupDriver.
    passive component SchAppDriver {

        @ Implement tick interface
        import Drv.Tick

        @ Port receiving routed cFS scheduler messages. Each received message
        @ triggers one tick on CycleOut.
        sync input port dataIn: Svc.ComDataWithContext

        @ Port returning ownership of buffers received on dataIn once processed
        output port dataReturnOut: Svc.ComDataWithContext

        ###############################################################################
        # Standard AC Ports: Required for Channels, Events, Commands, and Parameters  #
        ###############################################################################
        @ Port for requesting the current time
        time get port timeCaller

    }
}
