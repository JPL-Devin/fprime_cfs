####
# Unit-test-only override of CfsCmdRouterCfg.fpp providing a routing table
# that exercises all route categories.
####
register_fprime_config(
    CONFIGURATION_OVERRIDES
        "${CMAKE_CURRENT_LIST_DIR}/CfsCmdRouterCfg.fpp"
    INTERFACE
    DEPENDS
        FPrimeCfs_CfsCmdRouter_Types
)
