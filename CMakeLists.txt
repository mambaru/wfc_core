cmake_minimum_required(VERSION 3.10)

project(wfc_core VERSION "0.9.8")

include(cmake/ci.cmake)

env_option(WITH_PROMETHEUS "Build with prometheus module" OFF)

if ( WITH_PROMETHEUS )
  add_definitions(-DWITH_PROMETHEUS=${WITH_PROMETHEUS})
endif()

wci_submodule(NAME wfc SUPERMODULE)

if ( WITH_PROMETHEUS )
  find_package(prometheus-cpp QUIET)

  if(NOT prometheus-cpp_FOUND)
    wci_third_party(NAME prometheus-cpp SUPERMODULE)
    list(APPEND CMAKE_MODULE_PATH ${CMAKE_BINARY_DIR}/third_party/lib64/cmake/prometheus-cpp)
    include(prometheus-cpp-targets)
  endif()
endif()


enable_stat()

add_subdirectory(package)

if ( BUILD_TESTING )
  enable_testing()
  add_subdirectory(tests)
endif()

include(cmake/install.cmake)
