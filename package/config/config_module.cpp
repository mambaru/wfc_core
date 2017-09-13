
#include "config_module.hpp"
#include "config/config_singleton.hpp"
#include <wfc/module/component_list.hpp>
#include <wfc/name.hpp>

namespace wfc{ namespace core{

namespace {  
  
  WFC_NAME2(module_name, "config")

  class impl: public ::wfc::component_list
  <
    module_name,
    config_singleton
  >
  {
    virtual std::string description() const override
    {
      return "Оne-component module";
    }
  };

}

config_module::config_module()
  : module( std::make_shared<impl>() )
{}

}}
