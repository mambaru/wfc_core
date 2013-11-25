#add_custom_target(
#  config_build_info
#  COMMAND
#    /bin/bash ${CMAKE_SOURCE_DIR}/comet/build_info.sh config
#  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
#  COMMENT "Create build info files"
#  VERBATIM
#)
include(BuildInfo)

SET(
  comet_config_source_list
  config_module.cpp
  config.cpp
)

add_library(
  comet_config SHARED
  ${comet_config_source_list}
)

build_info(comet_config config)
#target_link_libraries(comet_config /tmp/config_build_info.o)
#add_dependencies(comet_config config_build_info)
