cmake_minimum_required(VERSION 3.20)
project (vmtrace CXX ASM_NASM)
set(CMAKE_CXX_STANDARD 14)

add_executable(vmtrace main.cpp)
add_executable(short-loop short-loop.asm)
