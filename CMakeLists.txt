cmake_minimum_required(VERSION 3.10.0)
project(Virtual_Processing_Unit VERSION 1.0.0 LANGUAGES C CXX)


add_subdirectory(Virtual_Processing_Unit)
add_subdirectory(Virtual_Fictional_Emulator)

include(CTest)
enable_testing()
