//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2013-2015
//
// Copyright: See COPYING file that comes with this distribution
//

#include "startup_singleton.hpp"
#include <wfc/module/singleton.hpp>
#include <wfc/module/instance.hpp>
#include <wfc/json.hpp>
#include "startup.hpp"
#include "startup_config_json.hpp"

namespace wfc{
  
JSON_NAME2(startup_singleton_name, "startup")

class startup_singleton_impl
  : public ::wfc::singleton<startup_singleton_name, wfc::instance<startup_domain>, startup_config_json>
{  
};

startup_singleton::startup_singleton()
  : component( std::make_shared<startup_singleton_impl>() )
{
}

}
