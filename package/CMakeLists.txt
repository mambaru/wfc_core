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

  logger/logger/aux.cpp
  logger/logger/writer.cpp
  logger/logger/logger.cpp
  logger/logger/logger_singleton.cpp
  logger/logger_module.cpp

  workflow/workflow_module.cpp
  workflow/workflow/workflow_multiton.cpp
  workflow/workflow/workflow_domain.cpp

  stat/stat_module.cpp
  stat/stat/stat_multiton.cpp
  stat/stat/stat_domain.cpp

)

add_library(
  wfc_core 
  ${wfc_core_source_list}
)

build_info(wfc_core core)

install(TARGETS wfc_core DESTINATION ${CMAKE_INSTALL_PREFIX})
