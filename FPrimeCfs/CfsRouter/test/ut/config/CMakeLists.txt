####
# Unit-test-only override of CfsRouterCfg.fpp providing a routing table
# that exercises all route categories.
####
register_fprime_config(
    CONFIGURATION_OVERRIDES
        "${CMAKE_CURRENT_LIST_DIR}/CfsRouterCfg.fpp"
    INTERFACE
    DEPENDS
        FPrimeCfs_CfsRouter_Types
)
