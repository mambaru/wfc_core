
#include "core_module.hpp"
#include "core/core_singleton.hpp"
#include <wfc/module/component_list.hpp>
#include <wfc/json.hpp>

namespace wfc{
  
namespace 
{
  JSON_NAME2(core_module_name, "core")
  class impl: public ::wfc::component_list<
    core_module_name,
    core_singleton
  >
  {};
}

core_module::core_module()
  : module( std::make_shared<impl>() )
{
}

}
