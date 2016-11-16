
#include "stat_module.hpp"
#include "stat/stat_multiton.hpp"
#include <wfc/module/component_list.hpp>
#include <wfc/name.hpp>

namespace wfc{
  
namespace
{
  WFC_NAME2(module_name, "stat")

  class impl: public ::wfc::component_list<
    module_name,
    stat_multiton
  >
  {};
}

stat_module::stat_module()
  : module( std::make_shared<impl>() )
{
}

}
