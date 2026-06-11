cmake_minimum_required(VERSION 3.22)
project(CFE_FPRIME_CFS C CXX)
register_fprime_project()
add_compile_options(-fPIC)


add_fprime_subdirectory("${CMAKE_CURRENT_LIST_DIR}/FPrimeCfs")