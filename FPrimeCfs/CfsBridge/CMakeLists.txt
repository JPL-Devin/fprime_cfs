####
# F Prime CMakeLists.txt:
#
# SOURCES: list of source files (to be compiled)
# AUTOCODER_INPUTS: list of files to be passed to the autocoders
# DEPENDS: list of libraries that this module depends on
#
# More information in the F´ CMake API documentation:
# https://fprime.jpl.nasa.gov/latest/docs/reference/api/cmake/API/
#
####
#add_fprime_subdirectory("${CMAKE_CURRENT_LIST_DIR}/Types/")

# Module names are derived from the path from the nearest project/library/framework
# root when not specifically overridden by the developer, i.e. the module defined by
# `MyProj/Some/Path/CMakeLists.txt` will be named `MyProj_Some_Path`.
include_directories("${CMAKE_CURRENT_LIST_DIR}/../../fsw/inc")
register_fprime_library(
    AUTOCODER_INPUTS
        "${CMAKE_CURRENT_LIST_DIR}/CfsBridge.fpp"
    SOURCES
        "${CMAKE_CURRENT_LIST_DIR}/CfsBridge.cpp"
#   DEPENDS
#       MyPackage_MyOtherModule
)
target_link_libraries(${FPRIME_CURRENT_MODULE} core_api)

### Unit Tests ###
# register_fprime_ut(
#     AUTOCODER_INPUTS
#         "${CMAKE_CURRENT_LIST_DIR}/CfsBridge.fpp"
#     SOURCES
#         "${CMAKE_CURRENT_LIST_DIR}/test/ut/CfsBridgeTestMain.cpp"
#         "${CMAKE_CURRENT_LIST_DIR}/test/ut/CfsBridgeTester.cpp"
#     DEPENDS
#         STest # For rules-based testing
#     UT_AUTO_HELPERS
# )
