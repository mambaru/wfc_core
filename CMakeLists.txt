cmake_minimum_required(VERSION 2.6)

project(comet_core)

include_directories(../comet)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_SOURCE_DIR}/comet/cmake")
#LIST (APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/comet/cmake")

add_subdirectory(core)
add_subdirectory(tests)