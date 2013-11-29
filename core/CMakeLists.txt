message(STATUS "MODULE PATH: ${CMAKE_MODULE_PATH}")

include(BuildInfo)

SET(
  comet_core_source_list
  detail/po.cpp  
  
  core_module.cpp
  core.cpp
)

add_library(
  comet_core SHARED
  ${comet_core_source_list}
)

build_info(comet_core core)
