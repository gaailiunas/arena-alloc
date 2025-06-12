cmake_minimum_required(VERSION 3.10)
project(example1)

find_package(arena_alloc REQUIRED)
add_executable(example1 main.cpp)
target_link_libraries(example1 PRIVATE arena_alloc::arena_alloc)
