//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2013-2018
//
// Copyright: See COPYING file that comes with this distribution
//

#pragma once

#include "config_config.hpp"
#include <wfc/json.hpp>

namespace wfc{ namespace core{

struct config_config_json
{
  JSON_NAME(reload_changed_ms)
  JSON_NAME(reload_sighup)
  
  typedef json::object<
    config_config,
    json::member_list<
        json::member<n_reload_sighup,  config_config, bool, &config_config::reload_sighup>,
        json::member<n_reload_changed_ms, config_config, time_t, &config_config::reload_changed_ms>
    >,
    json::strict_mode
  > type; 
  
  typedef type::target target;
  typedef type::serializer serializer;
  typedef type::member_list member_list; 

};

}}
