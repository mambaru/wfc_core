cmake_minimum_required(VERSION 2.6)

project(comet_logger)

include_directories(../comet)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

add_subdirectory(logger)
add_subdirectory(tests)