add_custom_target(
  core_build_info
  COMMAND
    /bin/bash ${CMAKE_SOURCE_DIR}/comet/build_info.sh > ${CMAKE_CURRENT_SOURCE_DIR}/build_info.h
  COMMENT
    "Create build info files"
  VERBATIM
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

SET(
  comet_core_source_list
  detail/po.cpp  
  core_module.cpp
  core.cpp
)

add_library(
  comet_core SHARED
  ${comet_core_source_list}
)

add_dependencies(
  comet_core
  core_build_info
)
