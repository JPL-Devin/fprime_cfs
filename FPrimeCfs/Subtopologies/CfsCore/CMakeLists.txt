if (NOT FPRIME_ENABLE_TEXT_LOGGERS)
    return()
endif()
add_fprime_subdirectory("${CMAKE_CURRENT_LIST_DIR}/CfsCoreConfig/")

register_fprime_module(
    EXCLUDE_FROM_ALL
    AUTOCODER_INPUTS
        "${CMAKE_CURRENT_LIST_DIR}/CfsCore.fpp"
    HEADERS
        "${CMAKE_CURRENT_LIST_DIR}/SubtopologyTopologyDefs.hpp"
        "${CMAKE_CURRENT_LIST_DIR}/PingEntries.hpp"
    INTERFACE
    DEPENDS
        FPrimeCfs_Subtopologies_CfsCore_CfsCoreConfig
)
