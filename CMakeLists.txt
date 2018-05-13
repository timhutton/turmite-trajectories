cmake_minimum_required(VERSION 3.10)

project(llrr)

set(CMAKE_CXX_STACK_SIZE "200000000")
set(CMAKE_CXX_STANDARD 17)

add_executable(llrr        llrr.cpp)
add_executable(walk_plot   walk_plot.cpp)
add_executable(chain_shape chain_shape.cpp)
add_executable(shape_test  shape_test.cpp)
