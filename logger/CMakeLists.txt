add_custom_target(
  logger_build_info
  COMMAND
    /bin/bash ${CMAKE_SOURCE_DIR}/comet/build_info.sh > ${CMAKE_CURRENT_SOURCE_DIR}/build_info.h
  COMMENT
    "Create build info files"
  VERBATIM
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

SET(
  comet_logger_source_list
  logger_module.cpp
  logger.cpp
)

add_library(
  comet_logger SHARED
  ${comet_logger_source_list}
)

add_dependencies(
  comet_logger
  logger_build_info
)
