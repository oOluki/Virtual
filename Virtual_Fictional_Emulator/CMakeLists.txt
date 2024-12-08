# Set the minimum version of CMake
cmake_minimum_required(VERSION 3.10.0)
project(Virtual_Fictional_Emulator VERSION 1.0.0 LANGUAGES C CXX)


add_executable(fiction src/compiler.c)
add_executable(vimulator src/virtual_emulator.c)

include_directories(examples)


include(CTest)
enable_testing()


file(GLOB TXT_FILES "examples/*.txt")

# Iterate over each file
foreach(FILE ${TXT_FILES})


    get_filename_component(FILENAME_WE ${FILE} NAME_WE)
    get_filename_component(FILENAME ${FILE} NAME)

    add_test(NAME "VFE_COMPILE_${FILENAME_WE}" COMMAND fiction ${FILE} ${CMAKE_BINARY_DIR}/${FILENAME}.out)
    add_test(NAME "VFE_DECOMPILE_${FILENAME_WE}" COMMAND fiction -r ${CMAKE_BINARY_DIR}/${FILENAME}.out)
    add_test(NAME "VFE_RUN_${FILENAME_WE}" COMMAND vimulator ${CMAKE_BINARY_DIR}/${FILENAME}.out)

endforeach()


