cmake_minimum_required(VERSION 2.6)

project(wfc_core)

set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_SOURCE_DIR}/wfc/cmake;${CMAKE_CURRENT_SOURCE_DIR}/../wfc/cmake")

include(Wfc)

include_directories(wfc)
include_directories(.)

add_subdirectory(core)
