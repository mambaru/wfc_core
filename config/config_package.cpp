
#include "config_package.hpp"
#include "config_module.hpp"
#include <wfc/module/module_list.hpp>
#include <iow/json/name.hpp>

namespace wfc{
  
JSON_NAME2(config_package_name, "config")

class config_package_impl: public ::wfc::module_list<
  config_package_name,
  config_module
>
{  
};

config_package::config_package()
  : package( std::make_shared<config_package_impl>() )
{
}

}
