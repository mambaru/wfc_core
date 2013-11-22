#pragma once

#include "core_config.hpp"
#include <comet/json/json.hpp>
#include <comet/json/name.hpp>

namespace mamba{ namespace comet{

struct config_config_json
{
  NAME(enabled)
  
  typedef json::object<
    config_config,
    type_list_n<
       json::member<n_enabled, core_config, bool, &core_config::enabled>
    >::type
  >::serializer serializer; 
};

}}
