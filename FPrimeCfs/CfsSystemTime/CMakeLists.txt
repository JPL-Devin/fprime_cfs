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

register_fprime_module(
    AUTOCODER_INPUTS
        "${CMAKE_CURRENT_LIST_DIR}/CfsSystemTime.fpp"
    SOURCES
        "${CMAKE_CURRENT_LIST_DIR}/CfsSystemTime.cpp"
    DEPENDS
        FPrimeCfs_Ports
)

if (TARGET core_api)
    # Building within a cFS mission build: link against the real cFE core API
    target_link_libraries(${FPRIME_CURRENT_MODULE} PUBLIC core_api)
else()
    # Standalone (unit test) build: use the cFE stub layer in test/ut/stubs
    target_include_directories(${FPRIME_CURRENT_MODULE} PUBLIC "${CMAKE_CURRENT_LIST_DIR}/test/ut/stubs")
endif()

### Unit Tests ###
register_fprime_ut(
    AUTOCODER_INPUTS
        "${CMAKE_CURRENT_LIST_DIR}/CfsSystemTime.fpp"
    SOURCES
        "${CMAKE_CURRENT_LIST_DIR}/test/ut/stubs/CfeTimeStubs.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/test/ut/CfsSystemTimeTestMain.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/test/ut/CfsSystemTimeTester.cpp"
    UT_AUTO_HELPERS
)
