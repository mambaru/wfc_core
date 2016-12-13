//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2013-2015
//
// Copyright: See COPYING file that comes with this distribution
//

#include "startup_module.hpp"
#include "startup/startup_singleton.hpp"
#include <wfc/module/component_list.hpp>
#include <wfc/name.hpp>

namespace wfc{ namespace core{
  
namespace
{
  WFC_NAME2(module_name, "startup")

  class impl: public ::wfc::component_list<
    module_name,
    startup_singleton
  >
  {};
}

startup_module::startup_module()
  : module( std::make_shared<impl>() )
{}

}}
