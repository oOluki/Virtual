# Set the minimum version of CMake
cmake_minimum_required(VERSION 3.10.0)
project(Virtual_Processing_Unit VERSION 1.4.3 LANGUAGES C CXX)


add_executable(compile src/compiler.c)
add_executable(decompile src/decompiler.c)
add_executable(VPU src/vpu.c)


