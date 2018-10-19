//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2013-2018
//
// Copyright: See COPYING file that comes with this distribution
//

#pragma once

#include "core_config.hpp"
#include <wfc/json.hpp>
#include <wfc/core/workflow_options_json.hpp>

namespace wfc{ namespace core{

struct common_workflow_options_json
{
  JSON_NAME(cpu)
  
  typedef json::object<
    common_workflow_options,
    json::member_list<
      json::base<workflow_options_basic_json>,
      json::member<n_cpu, common_workflow_options, std::set<int>, &common_workflow_options::cpu, json::array< std::set< json::value<int> > > >
    >,
    json::strict_mode
  > type;
  
  typedef type::target target;
  typedef type::serializer serializer;
  typedef type::member_list member_list;

};

struct core_config_json
{
  JSON_NAME(idle_timeout_ms)
  JSON_NAME(core_timeout_ms)
  JSON_NAME(rlimit_as_mb)
  
  JSON_NAME(disable_statistics)
  JSON_NAME(nocall_callback_abort)
  JSON_NAME(nocall_callback_show)
  JSON_NAME(double_callback_abort)
  JSON_NAME(double_callback_show)

  JSON_NAME2(n_wfc_cpu, "wfc-cpu")
  JSON_NAME2(n_sys_cpu, "sys-cpu")
  JSON_NAME2(n_workflow, "common-workflow")
  
  typedef json::object<
    core_config,
    json::member_list<
       json::member<n_core_timeout_ms, core_config, time_t, &core_config::core_timeout_ms>,
       json::member<n_idle_timeout_ms, core_config, time_t, &core_config::idle_timeout_ms>,
       json::member<n_rlimit_as_mb, core_config, size_t, &core_config::rlimit_as_mb >,
       json::member<n_disable_statistics, core_config, bool, &core_config::disable_statistics >,
       json::member<n_nocall_callback_abort, core_config, bool, &core_config::nocall_callback_abort >,
       json::member<n_nocall_callback_show, core_config, bool, &core_config::nocall_callback_show >,
       json::member<n_double_callback_abort, core_config, bool, &core_config::double_callback_abort >,
       json::member<n_double_callback_show, core_config, bool, &core_config::double_callback_show >,
       // json::member<n_datapool, core_config, ::iow::io::data_map_options, &core_config::datapool, ::iow::io::data_map_options_json >,
       json::member<n_workflow, core_config, common_workflow_options, &core_config::common_workflow, common_workflow_options_json >,
       json::member<n_wfc_cpu, core_config, std::set<int>, &core_config::wfc_cpu, json::array< std::set< json::value<int> > > >,
       json::member<n_sys_cpu, core_config, std::set<int>, &core_config::sys_cpu, json::array< std::set< json::value<int> > > >
    >,
    json::strict_mode
  > type;
  
  typedef type::target target;
  typedef type::serializer serializer;
  typedef type::member_list member_list;
};

}}
