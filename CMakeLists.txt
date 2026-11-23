cmake_minimum_required(VERSION 3.20)
project (vmtrace CXX ASM_NASM)
set(CMAKE_CXX_STANDARD 14)

add_executable(vmtrace main.cpp FIFO.cpp)
target_include_directories(vmtrace PRIVATE 
    "${CMAKE_CURRENT_SOURCE_DIR}/."
)
add_executable(short-loop short-loop.asm)
