//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2013-2015
//
// Copyright: See COPYING file that comes with this distribution
//

#include "startup_package.hpp"
#include "startup_module.hpp"
#include <wfc/module/module_list.hpp>
#include <iow/json/name.hpp>

namespace wfc{
  
JSON_NAME2(startup_package_name, "startup")

class startup_package_impl: public ::wfc::module_list<
  startup_package_name,
  startup_module
>
{  
};

startup_package::startup_package()
  : package( std::make_shared<startup_package_impl>() )
{
}

}
