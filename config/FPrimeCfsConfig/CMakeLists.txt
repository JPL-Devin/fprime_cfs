####
# fprime_cfs library configuration overrides.
#
# Overrides the fprime default ComCfg.fpp to add the cFS command function code
# to ComCfg.FrameContext so it can be carried through the framer chain to
# FPrimeCfs.CfsCmdFramer. Projects overriding ComCfg.fpp themselves must
# carry this field forward.
####
register_fprime_config(
    CONFIGURATION_OVERRIDES
        "${CMAKE_CURRENT_LIST_DIR}/ComCfg.fpp"
    INTERFACE
)
