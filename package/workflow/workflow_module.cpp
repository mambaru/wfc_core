
#include "workflow_module.hpp"
#include "workflow/workflow_multiton.hpp"
#include <wfc/module/component_list.hpp>
#include <wfc/name.hpp>

namespace wfc{ namespace core{
  
namespace
{
  WFC_NAME2(module_name, "workflow")

  class impl: public ::wfc::component_list<
    module_name,
    workflow_multiton
  >
  {
    virtual std::string description() const override
    {
      return "Оne-component module";
    }
  };
}

workflow_module::workflow_module()
  : module( std::make_shared<impl>() )
{
}

}}
