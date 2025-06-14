cmake_minimum_required(VERSION 3.10)
project(example3)

find_package(arena_alloc REQUIRED)
add_executable(example3 main.cpp)
target_link_libraries(example3 PRIVATE arena_alloc::arena_alloc)
