#pragma once

#include "core_config.hpp"
#include <wfc/json.hpp>
#include <wfc/core/workflow_options_json.hpp>

namespace wfc{

struct core_config_json
{
  JSON_NAME(idle_timeout_ms)
  JSON_NAME(core_timeout_ms)
  JSON_NAME(rlimit_as_mb)
  JSON_NAME(enable_callback_check)
  JSON_NAME2(n_workflow, "core-workflow")
  
  typedef json::object<
    core_config,
    fas::type_list_n<
       json::member<n_core_timeout_ms, core_config, time_t, &core_config::core_timeout_ms>,
       json::member<n_idle_timeout_ms, core_config, time_t, &core_config::idle_timeout_ms>,
       json::member<n_rlimit_as_mb, core_config, size_t, &core_config::rlimit_as_mb >,
       json::member<n_enable_callback_check, core_config, bool, &core_config::enable_callback_check >,
       json::member<n_workflow, core_config, workflow_options, &core_config::core_workflow, workflow_options_on_json >
    >::type
  > type;
  
  typedef type::target target;
  typedef type::serializer serializer;
  typedef type::member_list member_list;
};

}
