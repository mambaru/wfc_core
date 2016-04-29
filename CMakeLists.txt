cmake_minimum_required(VERSION 2.6)

project(wfc_core)

include_directories(.)

add_subdirectory(package)

if ( WFC_BUILD_ALL )
  #add_subdirectory(tests)
endif()

