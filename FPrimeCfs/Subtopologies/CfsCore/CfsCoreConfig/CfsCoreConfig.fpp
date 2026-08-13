module CfsCoreConfig {
    # Base ID for the CfsCore Subtopology, all components are offsets from this base ID
    constant BASE_ID = 0x01000000

    module QueueSizes {
        constant cmdDisp = 10
        constant events  = 10
        constant tlmSend = 10
        constant $health = 25
    }

    module StackSizes {
        constant cmdDisp = 64 * 1024
        constant events  = 64 * 1024
        constant tlmSend = 64 * 1024
    }

    # Priorities use cFS/OSAL semantics: lower number = more urgent.
    # Values sit below cFE core services but above background tasks.
    module Priorities {
        constant cmdDisp = 140
        constant $health = 150
        constant events  = 160
        constant tlmSend = 170
    }
}
