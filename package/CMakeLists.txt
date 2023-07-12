
SET(
  wfc_core_source_list

  core_package.cpp
  core/core_module.cpp
  core/core/core_singleton.cpp
  core/core/core.cpp

  startup/startup/parse_arguments.cpp
  startup/startup/startup.cpp
  startup/startup/startup_singleton.cpp
  startup/startup_module.cpp

  config/config_module.cpp
  config/config/config_singleton.cpp
  config/config/config.cpp

  logger/logger/logger.cpp
  logger/logger/logger_singleton.cpp
  logger/logger_module.cpp

  workflow/workflow_module.cpp
  workflow/workflow/workflow_multiton.cpp
  workflow/workflow/workflow_domain.cpp
)

if ( WFC_ENABLE_STAT )
  SET(
    wfc_core_source_list
    ${wfc_core_source_list}
    statistics/statistics_module.cpp

    statistics/statistics/statistics_multiton.cpp
    statistics/statistics/statistics_domain.cpp

    statistics/statistics_aggregator/aggregator_multiton.cpp
    statistics/statistics_aggregator/aggregator_domain.cpp

    statistics/statistics_log/statlog_multiton.cpp
    statistics/statistics_log/statlog_domain.cpp

    statistics/system_statistics/system_statistics_multiton.cpp
    statistics/system_statistics/system_statistics_domain.cpp
    statistics/system_statistics/impl/get_procstat.cpp

    statistics/statistics_gateway/statistics_gateway_multiton.cpp
    statistics/statistics_gateway/statistics_gateway_compact_multiton.cpp
    statistics/statistics_service/statistics_service_multiton.cpp
    statistics/statistics_service/statistics_service_compact_multiton.cpp
  )
endif()

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" )
  set_source_files_properties( core/core/core.cpp PROPERTIES COMPILE_FLAGS -Wno-zero-as-null-pointer-constant)
endif()

add_library(wfc_core ${wfc_core_source_list})

wci_targets(wfc_core)
enable_stat(wfc_core)

target_include_directories(wfc_core PUBLIC ${wfc_core_SOURCE_DIR})

target_link_libraries(wfc_core PUBLIC wfc)
target_compile_definitions(wfc_core PRIVATE -DBOOST_ASIO_NO_DEPRECATED)
build_info(TARGET wfc_core PREFIX core)

install(TARGETS wfc_core DESTINATION ${CMAKE_INSTALL_PREFIX})
