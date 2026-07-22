module FPrimeCfs {
    @ Rate group driver triggered by cFS scheduler (SCH) messages. Scheduler
    @ messages are cFS commands: the CfsRouter routes the scheduler APID via
    @ its cFS command route to this component's cfsCommandIn port. Each
    @ received message produces one tick on CycleOut, driving the
    @ Svc.RateGroupDriver.
    passive component SchAppDriver {

        @ Implement tick interface
        import Drv.Tick

        @ Port receiving routed cFS scheduler command messages from the
        @ CfsRouter's cFS command route. Each received message triggers one
        @ tick on CycleOut.
        sync input port cfsCommandIn: FPrimeCfs.CfsCommand

        @ Port returning ownership of buffers received on cfsCommandIn once
        @ processed. Connects to the router's buffer return input.
        output port bufferReturnOut: Fw.BufferSend

        ###############################################################################
        # Standard AC Ports: Required for Channels, Events, Commands, and Parameters  #
        ###############################################################################
        @ Port for requesting the current time
        time get port timeCaller

    }
}
