cmake_minimum_required(VERSION 3.10)
project(example2)

find_package(arena_alloc REQUIRED)
add_executable(example2 main.c)
target_link_libraries(example2 PRIVATE arena_alloc::arena_alloc)
