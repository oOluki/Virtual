# Set the minimum version of CMake
cmake_minimum_required(VERSION 3.10.0)
project(Virtual_Processing_Unit VERSION 0.1.1 LANGUAGES C CXX)

add_executable(virtual src/main.c)

add_compile_definitions(-DVERSION=\"${PROJECT_VERSION}\")
