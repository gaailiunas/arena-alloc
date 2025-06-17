cmake_minimum_required(VERSION 3.10)
project(arena_alloc)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)

add_library(arena_alloc INTERFACE)

add_library(arena_alloc::arena_alloc ALIAS arena_alloc)

target_include_directories(arena_alloc
    INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

install(TARGETS arena_alloc
    EXPORT arena_allocTargets
    INCLUDES DESTINATION include
)

install(DIRECTORY include/ DESTINATION include)

install(EXPORT arena_allocTargets
    FILE arena_allocTargets.cmake
    NAMESPACE arena_alloc::
    DESTINATION lib/cmake/arena_alloc
)

include(CMakePackageConfigHelpers)
configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/arena_allocConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/arena_allocConfig.cmake"
    INSTALL_DESTINATION lib/cmake/arena_alloc
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/arena_allocConfig.cmake"
    DESTINATION lib/cmake/arena_alloc
)

project(examples)
add_executable(example1 examples/1/main.cpp)
add_executable(example2 examples/2/main.c)
add_executable(example3 examples/3/main.cpp)

target_link_libraries(example1 PRIVATE arena_alloc::arena_alloc)
target_link_libraries(example2 PRIVATE arena_alloc::arena_alloc)
target_link_libraries(example3 PRIVATE arena_alloc::arena_alloc)

