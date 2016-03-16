
include(BuildInfo)

include_directories(.)

SET(
  wfc_startup_source_list
  startup/domain/parse_arguments.cpp
  startup/domain/startup.cpp
  startup/startup_singleton.cpp
  startup/startup_module.cpp
  startup_package.cpp
)

add_library(
  wfc_startup
  ${wfc_startup_source_list}
)

build_info(wfc_startup startup)

install(TARGETS wfc_startup DESTINATION ${CMAKE_INSTALL_PREFIX})