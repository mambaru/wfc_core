
#include "config_build_info.h"
#include "config_package.hpp"
#include "config_module.hpp"
#include <wfc/module/module_list.hpp>
#include <iow/json/name.hpp>

namespace wfc{
  
class config_package_impl: public ::wfc::module_list<
  config_build_info,
  config_module
>
{  
};

config_package::config_package()
  : package( std::make_shared<config_package_impl>() )
{
}

}
