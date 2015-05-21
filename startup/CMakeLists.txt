message(STATUS "MODULE PATH: ${CMAKE_MODULE_PATH}")

include(BuildInfo)

SET(
  wfc_startup_source_list
  startup.cpp
  startup_object.cpp
  startup_module.cpp
  startup_package.cpp
  detail/po.cpp  
  #detail/po.cpp  
  
  #startup_module.cpp
  #startup.cpp
)

add_library(
  wfc_startup
  ${wfc_startup_source_list}
)

build_info(wfc_startup startup)

install(TARGETS wfc_startup DESTINATION ${CMAKE_INSTALL_PREFIX})