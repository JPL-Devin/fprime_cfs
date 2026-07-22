####
# CMakeLists.txt:
#
# Sets up the fprime module build within CMake.
####
register_fprime_module(
    AUTOCODER_INPUTS
        "${CMAKE_CURRENT_LIST_DIR}/FPrimeCfsPorts.fpp"
    DEPENDS
        Fw_Buffer
        FPrimeCfs_CfsRouter_Types
)
