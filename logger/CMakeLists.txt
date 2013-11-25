#add_custom_target(
#  logger_build_info
#  COMMAND
#    /bin/bash ${CMAKE_SOURCE_DIR}/comet/build_info.sh logger
#  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
#  COMMENT "Create build info files"
#  VERBATIM
#)

SET(
  comet_logger_source_list
  logger_module.cpp
  logger.cpp
)

add_library(
  comet_logger SHARED
  ${comet_logger_source_list}
)

build_info(comet_logger logger)
#target_link_libraries(comet_logger /tmp/logger_build_info.o)
#add_dependencies(
#  comet_logger
#  logger_build_info
#)
