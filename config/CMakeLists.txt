add_custom_target(
  create_build_info
  COMMAND
    /bin/bash ${CMAKE_SOURCE_DIR}/../comet/build_info.sh > ${CMAKE_CURRENT_SOURCE_DIR}/build_info.h
  COMMENT
    "Create build info files"
  VERBATIM
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

SET(
  comet_config_source_list
  config_module.cpp
  config.cpp
)

add_library(
  comet_config SHARED
  ${comet_config_source_list}
)

add_dependencies( comet_config
  create_build_info
)
