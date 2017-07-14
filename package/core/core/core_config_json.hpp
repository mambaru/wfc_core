#pragma once

#include "core_config.hpp"
#include <wfc/json.hpp>
#include <iow/io/aux/data_pool_options_json.hpp>
#include <wfc/core/workflow_options_json.hpp>

namespace wfc{ namespace core{

struct core_config_json
{
  JSON_NAME(idle_timeout_ms)
  JSON_NAME(core_timeout_ms)
  JSON_NAME(rlimit_as_mb)
  JSON_NAME(enable_callback_check)
  JSON_NAME2(n_wfc_cpu, "wfc-cpu")
  JSON_NAME2(n_sys_cpu, "sys-cpu")
  JSON_NAME(datapool)
  JSON_NAME2(n_workflow, "core-workflow")
  
  typedef json::object<
    core_config,
    fas::type_list_n<
       json::member<n_core_timeout_ms, core_config, time_t, &core_config::core_timeout_ms>,
       json::member<n_idle_timeout_ms, core_config, time_t, &core_config::idle_timeout_ms>,
       json::member<n_rlimit_as_mb, core_config, size_t, &core_config::rlimit_as_mb >,
       json::member<n_enable_callback_check, core_config, bool, &core_config::enable_callback_check >,
       json::member<n_datapool, core_config, ::iow::io::data_map_options, &core_config::datapool, ::iow::io::data_map_options_json >,
       json::member<n_workflow, core_config, workflow_options, &core_config::core_workflow, workflow_options_on_json >,
       json::member<n_wfc_cpu, core_config, std::set<int>, &core_config::wfc_cpu, json::array< std::set< json::value<int> > > >,
       json::member<n_sys_cpu, core_config, std::set<int>, &core_config::sys_cpu, json::array< std::set< json::value<int> > > >
    >::type
  > type;
  
  typedef type::target target;
  typedef type::serializer serializer;
  typedef type::member_list member_list;
};

}}
