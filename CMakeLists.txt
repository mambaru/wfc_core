cmake_minimum_required(VERSION 2.6)

project(wfc_config)

set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_SOURCE_DIR}/wfc/cmake")

include(Wfc)

include_directories(wfc)

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
#set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_SOURCE_DIR}/wfc/cmake")

add_subdirectory(config)
add_subdirectory(tests)