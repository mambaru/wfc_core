cmake_minimum_required(VERSION 2.6)

project(wfc_logger)


set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_SOURCE_DIR}/wfc/cmake")

include(Wfc)

include_directories(wfc)

add_subdirectory(logger)
#add_subdirectory(tests)