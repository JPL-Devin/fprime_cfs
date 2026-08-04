####
# FPrimeCfs/CfsCmdRouter/config/CfsCmdRouterConfig/CMakeLists.txt:
#
# Sets a list of source files for cmake to process as part of autocoding.
####
register_fprime_config(
    AUTOCODER_INPUTS
        "${CMAKE_CURRENT_LIST_DIR}/CfsCmdRouterCfg.fpp"
    DEPENDS
        FPrimeCfs_CfsCmdRouter_Types
)
