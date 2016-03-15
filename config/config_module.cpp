
#include "config_module.hpp"
#include "config_singleton.hpp"
#include <wfc/module/component_list.hpp>
#include <wfc/json.hpp>

namespace wfc{

namespace {  
JSON_NAME2(config_module_name, "config")

class impl: public ::wfc::component_list<
  config_module_name,
  config_singleton
>
{  
};
}

config_module::config_module()
  : module( std::make_shared<impl>() )
{
}

  
}
