module FPrimeCfs {
    @ Text logger replacement that mirrors F Prime events to the cFS Event
    @ Services (EVS) subsystem. A drop-in replacement for Svc.PassiveTextLogger:
    @ connect component text event outputs to the TextLogger port. Each text
    @ event is sent to EVS via CFE_EVS_SendEvent, which handles console output
    @ and ground distribution like any other cFS application event.
    @
    @    -------------------------------
    @    | Event-Producing Components  |
    @    -------------------------------
    @         | (LogText)
    @    -------------
    @    | EvsMirror | ----> cFS EVS (CFE_EVS_SendEvent)
    @    -------------
    passive component EvsMirror {

        @ Text event input port. Connect component text event outputs here in
        @ place of Svc.PassiveTextLogger. Each event is sent to cFS EVS.
        sync input port TextLogger: Fw.LogText
    }
}
