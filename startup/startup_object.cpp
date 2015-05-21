//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2013-2015
//
// Copyright: See COPYING file that comes with this distribution
//

#include "startup_object.hpp"
#include <wfc/module/singleton.hpp>
#include <wfc/module/instance.hpp>
#include <wfc/json.hpp>
#include "startup.hpp"
#include "startup_config_json.hpp"

namespace wfc{
  
JSON_NAME2(startup_object_name, "startup")

class startup_object_impl
  : public ::wfc::singleton<startup_object_name, wfc::instance<startup_domain>, startup_config_json>
{  
};

startup_object::startup_object()
  : object( std::make_shared<startup_object_impl>() )
{
}

}
