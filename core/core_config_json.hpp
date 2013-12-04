#pragma once

#include "core_config.hpp"
#include <comet/json/json.hpp>
#include <comet/json/name.hpp>

namespace mamba{ namespace comet{

struct core_config_json
{
  NAME(enabled)
  NAME(idle_timeout_ms)
  NAME(wait_timeout_ms)
  
  typedef json::object<
    core_config,
    fas::type_list_n<
       json::member<n_enabled, core_config, bool, &core_config::enabled>,
       json::member<n_idle_timeout_ms, core_config, time_t, &core_config::idle_timeout_ms>,
       json::member<n_wait_timeout_ms, core_config, time_t, &core_config::wait_timeout_ms>
    >::type
  >::serializer serializer; 
};

}}
