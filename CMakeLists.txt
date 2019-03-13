cmake_minimum_required(VERSION 3.1)

project(wfc_core)

include(cmake/ci.cmake)

wfc_libs()

include_directories(.)

add_subdirectory(package)

if ( BUILD_TESTING )
  enable_testing()
  add_subdirectory(tests)
endif()
