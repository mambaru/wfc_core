
#include "workflow_module.hpp"
#include "workflow/workflow_multiton.hpp"
#include <wfc/module/component_list.hpp>
#include <wfc/name.hpp>

namespace wfc{
  
namespace
{
  WFC_NAME2(module_name, "workflow")

  class impl: public ::wfc::component_list<
    module_name,
    workflow_multiton
  >
  {};
}

workflow_module::workflow_module()
  : module( std::make_shared<impl>() )
{
}

}
