cmake_minimum_required(VERSION 2.6)

project(wfc_core)

if ( NOT WFC_READY )
  set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/wfc/cmake")
  include(Wfc)
  include_directories(wfc)
endif()

add_subdirectory(core)
