#pragma once

#include "core_config.hpp"
#include <wfc/json.hpp>

namespace wfc{

struct core_config_json
{
  JSON_NAME(idle_timeout_ms)
  JSON_NAME(rlimit_as_mb)
  JSON_NAME(enable_callback_check)
  
  typedef json::object<
    core_config,
    fas::type_list_n<
       json::member<n_idle_timeout_ms, core_config, time_t, &core_config::idle_timeout_ms>,
       json::member<n_rlimit_as_mb, core_config, size_t, &core_config::rlimit_as_mb >,
       json::member<n_enable_callback_check, core_config, bool, &core_config::enable_callback_check >
    >::type
  > type;
  
  typedef type::target target;
  typedef type::serializer serializer;
  typedef type::member_list member_list;
};

}
