cmake_minimum_required(VERSION 3.10)

project(llrr)

set(CMAKE_CXX_STACK_SIZE "200000000")
set(CMAKE_CXX_STANDARD 17)

add_executable(llrr        src/llrr.cpp)
add_executable(walk_plot   src/walk_plot.cpp)
add_executable(shape_test  src/shape_test.cpp)
