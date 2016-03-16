//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2013-2015
//
// Copyright: See COPYING file that comes with this distribution
//


#include <wfc/module/singleton.hpp>
#include <wfc/module/instance.hpp>
#include <wfc/json.hpp>

#include "startup_singleton.hpp"
#include "domain/startup.hpp"
#include "startup_config_json.hpp"

namespace wfc{

namespace
{  
  JSON_NAME2(name, "startup")

  class impl: public ::wfc::singleton<name, wfc::instance<startup_domain>, startup_config_json> {};
}

startup_singleton::startup_singleton()
  : component( std::make_shared<impl>() )
{
}

}
