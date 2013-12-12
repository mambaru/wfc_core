cmake_minimum_required(VERSION 2.6)

project(comet_core)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

include(Wfc)

include_directories(wfc)

add_subdirectory(core)
add_subdirectory(tests)