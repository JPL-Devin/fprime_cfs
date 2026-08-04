add_fprime_subdirectory("${CMAKE_CURRENT_LIST_DIR}/ComCfsConfig/")

register_fprime_module(
    EXCLUDE_FROM_ALL
    AUTOCODER_INPUTS
        "${CMAKE_CURRENT_LIST_DIR}/ComCfs.fpp"
    HEADERS
        "${CMAKE_CURRENT_LIST_DIR}/SubtopologyTopologyDefs.hpp"
        "${CMAKE_CURRENT_LIST_DIR}/PingEntries.hpp"
    DEPENDS
        FPrimeCfs_Subtopologies_ComCfs_ComCfsConfig
    INTERFACE
)
