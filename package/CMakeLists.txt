include(BuildInfo)

SET(
  wfc_core_source_list

  core_package.cpp
  core/core_module.cpp
  core/core/core_singleton.cpp
  core/core/core.cpp
  
  startup/startup/parse_arguments.cpp
  startup/startup/startup.cpp
  startup/startup/startup_singleton.cpp
  startup/startup_module.cpp
  
  config/config_module.cpp
  config/config/config_singleton.cpp
  config/config/config.cpp

)

add_library(
  wfc_core 
  ${wfc_core_source_list}
)

build_info(wfc_core core)

install(TARGETS wfc_core DESTINATION ${CMAKE_INSTALL_PREFIX})
