# Set the minimum version of CMake
cmake_minimum_required(VERSION 3.10.0)
project(Virtual_Processing_Unit VERSION 1.2.5 LANGUAGES C CXX)


add_executable(compile src/compiler.c)
add_executable(decompile src/decompiler.c)
add_executable(VPU src/vpu.c)

include_directories(examples)


include(CTest)
enable_testing()


file(GLOB TXT_FILES "examples/*.txt")
message("files: ${TXT_FILES}")

# Iterate over each file
foreach(FILE ${TXT_FILES})

    get_filename_component(FILENAME_WE ${FILE} NAME_WE)
    get_filename_component(FILENAME ${FILE} NAME)
    
    add_test(NAME "VPU_COMPILE_${FILENAME_WE}" COMMAND compile ${FILE} -o ${CMAKE_BINARY_DIR}/${FILENAME_WE}.out)
    add_test(NAME "VPU_DECOMPILE_${FILENAME_WE}" COMMAND decompile ${CMAKE_BINARY_DIR}/${FILENAME_WE}.out)
    add_test(NAME "VPU_RUN_${FILENAME_WE}" COMMAND VPU ${CMAKE_BINARY_DIR}/${FILENAME_WE}.out)

endforeach()


