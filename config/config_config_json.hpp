#pragma once

#include "config_config.hpp"
#include <comet/json/json.hpp>
#include <comet/json/name.hpp>

namespace mamba{ namespace comet{

struct config_config_json
{
  NAME(enabled)
  NAME(reload_changed)
  
  typedef json::object<
    config_config,
    type_list_n<
       json::member<n_enabled,        config_config, bool, &config_config::enabled>,
       json::member<n_reload_changed, config_config, bool, &config_config::reload_changed>
    >::type
  >::serializer serializer; 
};

}}
