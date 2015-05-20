cmake_minimum_required(VERSION 2.6)

project(wfc_startup)

if ( NOT CMAKE_MODULE_PATH )
  set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
  include_directories(wfc)
endif()

include(Wfc)

add_subdirectory(startup)

if ( WFC_BUILD_ALL )
  #add_subdirectory(tests)
endif()