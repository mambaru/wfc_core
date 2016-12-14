//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2013-2015
//
// Copyright: See COPYING file that comes with this distribution
//


#include <wfc/module/singleton.hpp>
#include <wfc/module/instance.hpp>
#include <wfc/json.hpp>

#include "startup_singleton.hpp"
#include "startup.hpp"
#include "startup_config_json.hpp"

namespace wfc{ namespace core{

namespace
{  
  JSON_NAME2(singleton_name, "startup")

  class impl : public ::wfc::singleton
  <
    singleton_name, 
    wfc::instance<startup_domain>,
    startup_config_json,
      component_features::DisabledEnabled
    | component_features::DisabledPriority 
    | component_features::DisabledSuspend 
    | component_features::DisabledWorkflow
    | component_features::DisabledStatistics
  > {};
}

startup_singleton::startup_singleton()
  : component( std::make_shared<impl>() )
{
}

}}
