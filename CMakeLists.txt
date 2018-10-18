cmake_minimum_required(VERSION 3.1)

project(wfc_core)

if ( NOT WFC_READY)
  set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
  include(mambaru)
  set(CMAKE_MODULE_PATH "${WFC_DIR}/cmake")
endif()

include_directories(.)

add_subdirectory(package)

if ( WFC_BUILD_ALL )
  #add_subdirectory(tests)
endif()

