#add_custom_target(
#  config_build_info
#  COMMAND
#    /bin/bash ${CMAKE_SOURCE_DIR}/wfc/build_info.sh config
#  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
#  COMMENT "Create build info files"
#  VERBATIM
#)
include(BuildInfo)

SET(
  wfc_config_source_list
  config_package.cpp
  config_module.cpp
  config_singleton.cpp
  config.cpp
)

add_library(
  wfc_config
  ${wfc_config_source_list}
)

build_info(wfc_config config)

install(TARGETS wfc_config DESTINATION ${CMAKE_INSTALL_PREFIX})
