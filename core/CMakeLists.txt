message(STATUS "MODULE PATH: ${CMAKE_MODULE_PATH}")
include(BuildInfo)

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

# MACRO(build_info target_name prefix)
#  add_custom_target(
#    ${prefix}_build_info
#    COMMAND
#      /bin/bash ${CMAKE_SOURCE_DIR}/comet/build_info.sh ${prefix}
#    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
#    COMMENT "Create build info files"
#    VERBATIM
#  )
#  add_dependencies(${target_name} ${prefix}_build_info)
#  target_link_libraries(${target_name} /tmp/${prefix}_build_info.o)
# ENDMACRO(build_info)

build_info(comet_core core)
