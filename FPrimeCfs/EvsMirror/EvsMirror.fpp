module FPrimeCfs {
    @ Pass-through event mirror for the cFS Event Services (EVS) subsystem. This
    @ component sits between event-producing components and the Svc.EventManager:
    @ events received on logIn are forwarded unchanged on logOut, and text events
    @ received on textLogIn are forwarded unchanged on textLogOut. Each event is
    @ additionally mirrored to cFS EVS via CFE_EVS_SendEvent, using the formatted
    @ text when text logging is enabled and a compact identifier form otherwise.
    @
    @    -------------------------------
    @    | Event-Producing Components  |
    @    -------------------------------
    @         | (Log / LogText)
    @    -------------
    @    | EvsMirror | ----> cFS EVS (CFE_EVS_SendEvent)
    @    -------------
    @         |
    @    ------------------------------------
    @    | Svc.EventManager / Text Logger   |
    @    ------------------------------------
    passive component EvsMirror {

        @ Event input port. Events are forwarded unchanged on logOut. When text
        @ logging is disabled, events are mirrored to EVS from this port in a
        @ compact identifier form.
        sync input port logIn: Fw.Log

        @ Forwarded events. Connect to the Svc.EventManager event input port.
        output port logOut: Fw.Log

        @ Text event input port. Text events are forwarded unchanged on
        @ textLogOut. When text logging is enabled, events are mirrored to EVS
        @ from this port using the formatted event text.
        sync input port textLogIn: Fw.LogText

        @ Forwarded text events. Connect to the text logger (e.g.
        @ Svc.PassiveTextLogger) if console text logging is also desired.
        output port textLogOut: Fw.LogText
    }
}
