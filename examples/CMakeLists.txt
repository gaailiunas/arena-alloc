cmake_minimum_required(VERSION 3.10)
project(example)

find_package(arena_alloc REQUIRED)
add_executable(example main.cpp)
target_link_libraries(example PRIVATE arena_alloc::arena_alloc)
