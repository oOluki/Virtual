# Set the minimum version of CMake
cmake_minimum_required(VERSION 3.10.0)
project(Virtual_Processing_Unit VERSION 1.3.6 LANGUAGES C CXX)


add_executable(compile src/compiler.c)
add_executable(decompile src/decompiler.c)
add_executable(VPU src/vpu.c)

set_target_properties(compile PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
set_target_properties(decompile PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
set_target_properties(VPU PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")


