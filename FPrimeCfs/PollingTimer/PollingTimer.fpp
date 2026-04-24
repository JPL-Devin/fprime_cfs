module Svc {
    @ Polling timer based rate group driver
    passive component PollingTimer {

        @ Implement tick interface
        import Drv.Tick

        ###############################################################################
        # Standard AC Ports: Required for Channels, Events, Commands, and Parameters  #
        ###############################################################################
        @ Port for requesting the current time
        time get port timeCaller

    }
}