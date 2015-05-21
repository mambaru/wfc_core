//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2013-2015
//
// Copyright: See COPYING file that comes with this distribution
//


#pragma once

#include "startup_config.hpp"
#include <wfc/json.hpp>

namespace wfc{

struct startup_config_json
{
  JSON_NAME(enabled)
  JSON_NAME(idle_timeout_ms)
  JSON_NAME(wait_timeout_ms)
  
  typedef json::object<
    startup_config,
    fas::type_list_n<
       json::member<n_enabled, startup_config, bool, &startup_config::enabled>,
       json::member<n_idle_timeout_ms, startup_config, time_t, &startup_config::idle_timeout_ms>,
       json::member<n_wait_timeout_ms, startup_config, time_t, &startup_config::wait_timeout_ms>
    >::type
  > type;
  typedef type::serializer serializer;
  typedef type::member_list member_list;
  typedef type::target target;

};

}
