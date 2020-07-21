cmake_minimum_required(VERSION 3.1)

project(wfc_core)

include(cmake/ci.cmake)

if ( WCI_VERSION )
  option(WFC_ENABLE_STAT "Enable code for statistics" ON)
else()
  option(WFC_ENABLE_STAT "Enable code for statistics" OFF)
endif()

wci_getlib(NAME wfc ${WCI_SUPERMODULE})

add_subdirectory(package)

if ( BUILD_TESTING )
  enable_testing()
  add_subdirectory(tests)
endif()
