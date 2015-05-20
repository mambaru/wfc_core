
#include "config_module.hpp"
#include "config_object.hpp"
#include <wfc/module/object_list.hpp>
#include <wfc/json.hpp>

namespace wfc{
  
JSON_NAME2(config_module_name, "config")

class config_module_impl: public ::wfc::object_list<
  config_module_name,
  config_object
>
{  
};

config_module::config_module()
  : module( std::make_shared<config_module_impl>() )
{
}

  
}
