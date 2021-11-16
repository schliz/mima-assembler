cmake_minimum_required(VERSION 3.20)
project(mima_assembler C)

set(CMAKE_C_STANDARD 99)

add_executable(miasm src/assembler/main.c src/opcodes.h src/assembler/util.c src/assembler/util.h src/assembler/map.c src/assembler/map.h)
add_executable(midas src/disassembler/main.c src/opcodes.h)
add_executable(runmima src/interpreter/main.c src/opcodes.h)
