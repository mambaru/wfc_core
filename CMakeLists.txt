cmake_minimum_required(VERSION 3.10)

project(wfc_core)

include(cmake/ci.cmake)

wci_submodule(NAME wfc SUPERMODULE)
enable_stat()
add_subdirectory(package)

if ( BUILD_TESTING )
  enable_testing()
  add_subdirectory(tests)
endif()
