# Set the minimum version of CMake
cmake_minimum_required(VERSION 3.10.0)
project(Virtual_Processing_Unit VERSION 1.4.6 LANGUAGES C CXX)


add_executable(assemble src/assembler.c)
add_executable(disassemble src/disassembler.c)
add_executable(VPU src/vpu.c)


