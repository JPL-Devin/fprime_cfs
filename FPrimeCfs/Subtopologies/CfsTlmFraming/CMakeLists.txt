add_fprime_subdirectory("${CMAKE_CURRENT_LIST_DIR}/CfsTlmFramingConfig/")

register_fprime_module(
    EXCLUDE_FROM_ALL
    AUTOCODER_INPUTS
        "${CMAKE_CURRENT_LIST_DIR}/CfsTlmFraming.fpp"
    DEPENDS
        FPrimeCfs_Subtopologies_CfsTlmFraming_CfsTlmFramingConfig
    INTERFACE
)
