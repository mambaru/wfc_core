#pragma once

#include "config_config.hpp"
#include <wfc/json.hpp>

namespace wfc{

struct config_config_json
{
  //JSON_NAME(enabled)
  JSON_NAME(reload_changed)
  JSON_NAME(reload_sighup)
  
  typedef json::object<
    config_config,
    fas::type_list_n<
       json::member<n_reload_sighup,  config_config, bool, &config_config::reload_sighup>,
       json::member<n_reload_changed, config_config, bool, &config_config::reload_changed>
    >::type
  > type; 
  
  typedef type::target target;
  typedef type::serializer serializer;
  typedef type::member_list member_list; 

};

}
