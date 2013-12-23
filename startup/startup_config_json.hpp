#pragma once

#include "startup_config.hpp"
#include <wfc/json/json.hpp>
#include <wfc/json/name.hpp>

namespace wfc{

struct startup_config_json
{
  FAS_NAME(enabled)
  FAS_NAME(idle_timeout_ms)
  FAS_NAME(wait_timeout_ms)
  
  typedef json::object<
    startup_config,
    fas::type_list_n<
       json::member<n_enabled, startup_config, bool, &startup_config::enabled>,
       json::member<n_idle_timeout_ms, startup_config, time_t, &startup_config::idle_timeout_ms>,
       json::member<n_wait_timeout_ms, startup_config, time_t, &startup_config::wait_timeout_ms>
    >::type
  >::serializer serializer; 
};

}
