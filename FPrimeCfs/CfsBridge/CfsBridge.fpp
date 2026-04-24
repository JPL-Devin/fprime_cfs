module FPrimeCfs {
    @ Bridge component between the cFS software bus (SB) and the F Prime framework. This component effectively peforms
    @ the role of three software components:
    @   1. A "Framer" framinng messages in cFS SB format
    @   2. A "Deframer" deframinng messages from cFS SB format
    @   3. A "ComDriver" for sending and receiving messages over the cFS SB
    @
    @ Standard data provided to/from the F Prime framework is in the standard form for F Prime applications i.e.
    @ unframed F Prime packets (Fw::ComBuffer/Fw::Buffer). Non-standard APID payloads will be provided as-is for
    @ routing to custom application layer handlers
    @
    @    ------------------------------------
    @    | F Prime Application              |
    @    ------------------------------------
    @           |                     |
    @    -------------          -------------
    @    | ComQueue  |          |   Router  |
    @    -------------          -------------
    @                \          /
    @               -------------
    @               | CfsBridge |
    @               -------------
    @                     |
    @                     |
    @         -------- CFS SB  ----------
    @                                     
    # Note: F Prime flavored lollipops are sweet!
    queued component CfsBridge {
        # The cFS bridge component acts as a "Deframer" in that it deframes messages from the cFS softwar bus and
        # passes them to the F prime framework for routing.
        #
        # Note: this is only a partial implementation of the Deframer interface as it does not use F Prime for the
        # receipt of the data.

        #### Deframer Ports ####
        
        @ Port sending deframed cFS messages to the F Prime framework. The data will be the F Prime application layer
        @ message buffer. Context will contain the APID as derivied from the cFS message header.
        output port dataOut: Svc.ComDataWithContext

        @ Port to return deframed cFS messages data and context back to the cFS bridge component once F Prime has
        @ finished thus completing the data ownership transfer back to the cFS bridge component.
        sync input port dataReturnIn: Svc.ComDataWithContext

        @ Since the cFS bridge may be paired with a framer stack, it must accept com status signals
        sync input port comStatusIn: Fw.SuccessCondition

        # The cFS bridge component also acts as a "Framer" in that it will frame incoming messages from the F Prime
        # framework and send them out over the cFS software bus.
        #
        # Note: this is only a partial implementation of the Framer interface as it does not use F Prime for the
        # sending of data.

        #### Framer Ports ####

        @ Port to receive data to frame in a cFS message and send to the cFS software bus. The data will be in the F
        @ Prime application layer message buffer.
        async input port dataIn: Svc.ComDataWithContext

        @ Port for returning ownership of the incoming Fw::Buffer to its sender once framing is handled. This completes
        @ the data ownership transfer back to the F Prime framework.
        output port dataReturnOut: Svc.ComDataWithContext

        @ Since the cFS bridge may be paired with a comQueue, it must properly respect the com status signals.
        output port comStatusOut: Fw.SuccessCondition
    }
}
