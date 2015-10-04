//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2013-2015
//
// Copyright: See COPYING file that comes with this distribution
//

#include "startup_module.hpp"
#include "startup_singleton.hpp"
#include <wfc/module/component_list.hpp>
#include <wfc/json.hpp>

namespace wfc{
  
JSON_NAME2(startup_module_name, "startup")

class startup_module_impl: public ::wfc::component_list<
  startup_module_name,
  startup_singleton
>
{  
};

startup_module::startup_module()
  : module( std::make_shared<startup_module_impl>() )
{}

}
