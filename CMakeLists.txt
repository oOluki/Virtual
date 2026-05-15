# Set the minimum version of CMake
cmake_minimum_required(VERSION 3.10.0)
project(Virtual_Processing_Unit VERSION 0.1.2 LANGUAGES C CXX)

add_executable(virtual src/main.c
    src/assembler.c
    src/debugger.c
    src/labels.c
    src/main.c
    src/system.c
    src/virtual_files.c
    src/core.c
    src/disassembler.c
    src/lexer.c
    src/parser.c
    src/virtual.c
)

add_executable(vcc ccompiler/main.c
    ccompiler/statements.c
    ccompiler/expressions.c
    ccompiler/tokenizer.c
    ccompiler/parser.c
    ccompiler/builder.c)

add_compile_definitions(-DVERSION=\"${PROJECT_VERSION}\")
