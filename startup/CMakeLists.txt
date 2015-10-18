
include(BuildInfo)

SET(
  wfc_startup_source_list
  startup.cpp
  startup_singleton.cpp
  startup_module.cpp
  startup_package.cpp
  domain/parse_arguments.cpp
)

add_library(
  wfc_startup
  ${wfc_startup_source_list}
)

build_info(wfc_startup startup)

install(TARGETS wfc_startup DESTINATION ${CMAKE_INSTALL_PREFIX})