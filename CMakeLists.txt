cmake_minimum_required(VERSION 3.1)

project(wfc_core)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
include(ci)

if ( STANDALONE )
  getlibs(faslib wjson wlog wflow iow wjrpc wrtstat wfc)
  list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/external/wfc/cmake)
endif()

include_directories(.)

add_subdirectory(package)

if ( BUILD_TESTING )
  #add_subdirectory(tests)
endif()
