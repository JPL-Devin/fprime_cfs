module FPrimeCfs {
    @ Bridge component between the cFS software bus (SB) and the F Prime framework. This component performs the role
    @ of a "ComDriver" at the space packet layer: it sends and receives complete CCSDS space packets over the cFS SB.
    @
    @ Data provided to the bridge for transmission is one or more complete CCSDS space packets (e.g. from an
    @ Svc.Ccsds.SpacePacketFramer, possibly concatenated by an Svc.ComAggregator); each packet is transmitted on the
    @ software bus as its own message. Data received from the software bus is emitted as the complete cFS message
    @ (a CCSDS space packet) for deframing by a downstream deframer (e.g. Svc.Ccsds.SpacePacketDeframer) before
    @ routing.
    @
    @    ------------------------------------
    @    | F Prime Application              |
    @    ------------------------------------
    @           |                     |
    @    -------------          -------------
    @    | ComQueue  |          |   Router  |
    @    -------------          -------------
    @          |                      |
    @          |               -------------
    @          |               | Deframer  |
    @          |               -------------
    @           \               /
    @               -------------
    @               | CfsBridge |
    @               -------------
    @                     |
    @                     |
    @         -------- CFS SB  ----------
    queued component CfsBridge {
        # The cFS bridge component acts as a "ComDriver" in that it receives messages from the cFS software bus and
        # passes them to the F Prime framework for deframing (e.g. by an Svc.Ccsds.SpacePacketDeframer) and routing.

        #### Receive Ports ####

        @ Port sending received cFS messages to the F Prime framework. The data will be the complete cFS message
        @ (a CCSDS space packet including primary and any secondary headers) for downstream deframing. The context
        @ is defaulted; downstream deframers derive the APID and other fields from the packet headers.
        output port dataOut: Svc.ComDataWithContext

        @ Port to return received cFS message data and context back to the cFS bridge component once F Prime has
        @ finished thus completing the data ownership transfer back to the cFS bridge component. When bufferAllocate
        @ is connected, the returned buffer is deallocated via bufferDeallocate.
        sync input port dataReturnIn: Svc.ComDataWithContext

        @ Port for allocating a buffer to copy each received software bus message into before emitting it on
        @ dataOut. When connected, dataOut buffers are owned by the allocator and remain valid until returned on
        @ dataReturnIn. When unconnected, received messages are emitted zero-copy: the buffer aliases software bus
        @ memory valid only until the next poll, so all downstream consumers must return it synchronously.
        output port bufferAllocate: Fw.BufferGet

        @ Port for deallocating buffers allocated via bufferAllocate once they are returned on dataReturnIn
        output port bufferDeallocate: Fw.BufferSend

        @ Since the cFS bridge may be paired with a framer stack, it must accept com status signals
        sync input port comStatusIn: Fw.SuccessCondition

        @ Scheduled input driving the bridge: each invocation drains the component's message
        @ queue and polls the cFS software bus once (equivalent to one process() call).
        @ Deployments may drive the bridge from a rate group via this port instead of
        @ calling process() from a dedicated loop.
        sync input port schedIn: Svc.Sched

        # The cFS bridge component also acts as the sending "ComDriver": it transmits framed space packets from the
        # F Prime framework out over the cFS software bus.

        #### Send Ports ####

        @ Port to receive data to send to the cFS software bus. The data will be one or more complete CCSDS space
        @ packets (e.g. from an Svc.Ccsds.SpacePacketFramer, possibly concatenated by an Svc.ComAggregator); each
        @ packet is transmitted on the software bus as its own message.
        async input port dataIn: Svc.ComDataWithContext

        @ Port for returning ownership of the incoming Fw::Buffer to its sender once framing is handled. This completes
        @ the data ownership transfer back to the F Prime framework.
        output port dataReturnOut: Svc.ComDataWithContext

        @ Since the cFS bridge may be paired with a comQueue, it must properly respect the com status signals.
        output port comStatusOut: Fw.SuccessCondition
    }
}
