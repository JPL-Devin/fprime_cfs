####
# F Prime CMakeLists.txt:
#
# Registers the cFE ES child task implementation of Os::Task (Os_Task_Cfs).
# Select it in a deployment with:
#
#     fprime_target_implementations("<deployment>" Os_Task_Cfs)
#
####

register_os_implementation("Task" Cfs Fw_Logger)

if (TARGET core_api)
    # Building within a cFS mission build: link against the real cFE core API
    target_link_libraries(Os_Task_Cfs_Implementation PUBLIC core_api)
else()
    # Standalone (unit test) build: use the cFE stub layer in test/ut/stubs
    target_include_directories(Os_Task_Cfs_Implementation PUBLIC "${CMAKE_CURRENT_LIST_DIR}/test/ut/stubs")
endif()

### Unit Tests ###
register_fprime_ut(
    FPrimeCfs_Os_Cfs_Task_ut_exe
  SOURCES
    "${CMAKE_CURRENT_LIST_DIR}/test/ut/CfsTaskTestMain.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/test/ut/stubs/CfeEsStubs.cpp"
  DEPENDS
    Fw_Types
    Fw_Time
    Fw_Logger
    Os
  CHOOSES_IMPLEMENTATIONS
    Os_Task_Cfs
)
