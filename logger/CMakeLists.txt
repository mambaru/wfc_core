#add_custom_target(
#  logger_build_info
#  COMMAND
#    /bin/bash ${CMAKE_SOURCE_DIR}/wfc/build_info.sh logger
#  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
#  COMMENT "Create build info files"
#  VERBATIM
#)

include(BuildInfo)

SET(
  wfc_logger_source_list
  logger/writer.cpp
  logger/logger.cpp
  module/logger_object.cpp
  module/logger_module.cpp
  logger_package.cpp
)

include_directories(.)

add_library(
  wfc_logger
  ${wfc_logger_source_list}
)

build_info(wfc_logger logger)

install(TARGETS wfc_logger DESTINATION ${CMAKE_INSTALL_PREFIX})
