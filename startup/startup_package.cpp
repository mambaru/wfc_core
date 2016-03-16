//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2013-2015
//
// Copyright: See COPYING file that comes with this distribution
//

#include <wfc/module/module_list.hpp>
#include <iow/json/name.hpp>

#include "startup_build_info.h"
#include "startup_package.hpp"
#include "startup/startup_module.hpp"

namespace wfc{
  
namespace 
{
  class impl: public ::wfc::module_list<
    startup_build_info,
      startup_module
  >
  {  
  };
}

startup_package::startup_package()
  : package( std::make_shared<impl>() )
{
}

}
