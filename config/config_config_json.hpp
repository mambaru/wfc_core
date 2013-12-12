#pragma once

#include "config_config.hpp"
#include <wfc/json/json.hpp>
#include <wfc/json/name.hpp>

namespace wfc{

struct config_config_json
{
  NAME(enabled)
  NAME(reload_changed)
  
  typedef json::object<
    config_config,
    fas::type_list_n<
       json::member<n_enabled,        config_config, bool, &config_config::enabled>,
       json::member<n_reload_changed, config_config, bool, &config_config::reload_changed>
    >::type
  >::serializer serializer; 
};

}
