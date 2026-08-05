module ComCfsConfig {
    # Base ID for the ComCfs Subtopology, all components are offsets from this base ID
    constant BASE_ID = 0x02000000

    module QueueSizes {
        constant cfsBridge = 10
    }

    # cFS bridge constants
    module Bridge {
        constant pipeDepth = 10
    }

    # Buffer management constants
    module BuffMgr {
        constant commsBuffSize      = 2048
        constant commsBuffCount     = 20
        constant commsFileBuffSize  = 3000
        constant commsFileBuffCount = 30
        constant commsBuffMgrId     = 200
    }
}
