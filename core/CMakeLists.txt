SET(
  comet_core_source_list
  core.cpp
  core_po.cpp
  core_module.cpp
)

add_library(
  comet_core SHARED
  ${comet_core_source_list}
)
