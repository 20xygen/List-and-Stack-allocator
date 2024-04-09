cmake_minimum_required(VERSION 3.26)
project(list_and_stack_allocator)

set(CMAKE_CXX_STANDARD 17)

add_executable(list_and_stack_allocator main.cpp
        stackallocator.h)
