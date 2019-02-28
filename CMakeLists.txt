cmake_minimum_required(VERSION 3.1)

project(wfc_core)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
include(ci)

wfc_libs()

include_directories(.)

add_subdirectory(package)

if ( BUILD_TESTING )
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${UNIT_TEST_BIN_OUTPUT_DIR})                                                                                                                 
  #add_subdirectory(tests)
endif()
