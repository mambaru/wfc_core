
#include "core_module.hpp"
#include "core/core_singleton.hpp"
#include <wfc/module/component_list.hpp>
#include <wfc/name.hpp>

namespace wfc{
  
namespace 
{
  WFC_NAME2(module_name, "core")

  class impl: public ::wfc::component_list
  <
    module_name,
    core_singleton
  >
  {};
}

core_module::core_module()
  : module( std::make_shared<impl>() )
{
}

}
