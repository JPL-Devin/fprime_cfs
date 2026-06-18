cmake_minimum_required(VERSION 3.22)
project(CFE_FPRIME_CFS C CXX)
register_fprime_project()

# In this library, we add -fPIC to the global interface target ... so everything compiles with -fPIC. This is needed
# because cFS builds apps as shared libraries. Only the final app should be shared (not intermediate F Prime
# libraries). This means we cannot get away with just setting BUILD_SHARED_LIBS to ON.
target_compile_options("${FPRIME_GLOBAL_INTERFACE_TARGET}" INTERFACE -fPIC)

add_fprime_subdirectory("${CMAKE_CURRENT_LIST_DIR}/FPrimeCfs")
