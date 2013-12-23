message(STATUS "MODULE PATH: ${CMAKE_MODULE_PATH}")

include(BuildInfo)

SET(
  wfc_startup_source_list
  detail/po.cpp  
  
  startup_module.cpp
  startup.cpp
)

add_library(
  wfc_startup SHARED
  ${wfc_startup_source_list}
)

build_info(wfc_startup startup)
