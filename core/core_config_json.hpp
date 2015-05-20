#pragma once

#include "core_config.hpp"
#include <wfc/json.hpp>

namespace wfc{

struct core_config_json
{
  //JSON_NAME(enabled)
  JSON_NAME(idle_timeout_ms)
  // JSON_NAME(wait_timeout_ms)
  JSON_NAME(rlimit_as_mb)
  
  typedef json::object<
    core_config,
    fas::type_list_n<
       // json::member<n_enabled, core_config, bool, &core_config::enabled>,
       json::member<n_idle_timeout_ms, core_config, time_t, &core_config::idle_timeout_ms>,
       // json::member<n_wait_timeout_ms, core_config, time_t, &core_config::wait_timeout_ms>,
       json::member<n_rlimit_as_mb, core_config, size_t, &core_config::rlimit_as_mb >
       
    >::type
  > type;
  typedef type::target target;
  typedef type::serializer serializer;
  typedef type::member_list member_list;
};

}
