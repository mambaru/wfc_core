cmake_minimum_required(VERSION 3.10)

project(wfc_core VERSION "0.9.8")

include(cmake/ci.cmake)

env_option(WITH_PROMETHEUS "Build with prometheus module" OFF)

if ( WITH_PROMETHEUS )
  add_definitions(-DWITH_PROMETHEUS=${WITH_PROMETHEUS})
endif()

wci_submodule(NAME wfc SUPERMODULE)

if ( WITH_PROMETHEUS )
  include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/prometheus_deps.cmake)
  wfc_prometheus_ensure_deps()

  find_package(prometheus-cpp QUIET)

  if(NOT prometheus-cpp_FOUND)
    # pull only; civetweb — единственный submodule (без googletest / SUPERMODULE)
    wfc_prometheus_build_and_import()
  endif()
endif()


enable_stat()

add_subdirectory(package)

if ( BUILD_TESTING )
  enable_testing()
  add_subdirectory(tests)
endif()

include(cmake/install.cmake)
