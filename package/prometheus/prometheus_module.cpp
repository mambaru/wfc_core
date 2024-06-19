
#include "prometheus_module.hpp"
#include "prometheus/prometheus_multiton.hpp"
#include <wfc/module/component_list.hpp>
#include <wfc/name.hpp>

namespace wfc{ namespace core{
  
namespace
{
  WFC_NAME2(module_name, "prometheus")

  class impl: public ::wfc::component_list<
    module_name,
    prometheus_multiton
  >
  {
    virtual std::string description() const override
    {
      return "Оne-component module";
    }
  };
}

prometheus_module::prometheus_module()
  : module( std::make_shared<impl>() )
{
}

}}
