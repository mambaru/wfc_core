message(STATUS "MODULE PATH: ${CMAKE_MODULE_PATH}")

include(BuildInfo)

SET(
  wfc_core_source_list
#detail/po.cpp  
  
  core_module.cpp
  core.cpp
)

add_library(
  wfc_core SHARED
  ${wfc_core_source_list}
)

build_info(wfc_core core)

install(TARGETS wfc_core DESTINATION ${CMAKE_INSTALL_PREFIX})