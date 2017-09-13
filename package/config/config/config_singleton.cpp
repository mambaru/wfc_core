
#include "config_singleton.hpp"
#include <wfc/module/singleton.hpp>
#include <wfc/module/instance.hpp>
#include <wfc/name.hpp>
#include "config.hpp"
#include "config_config_json.hpp"

namespace wfc{  namespace core{
  
namespace{

WFC_NAME2(config_singleton_name, "config")

class impl: public ::wfc::singleton<
  config_singleton_name,
  wfc::instance<config>,
  config_config_json,
    component_features::DisabledSuspend
  | component_features::DisabledWorkflow
  | component_features::DisabledPriority
>
{  
public:
  virtual std::string interface_name() const override
  {
    return std::string("wfc::iconfig");
  }

  virtual std::string description() const override
  {
    return std::string("Сonfiguration and dynamic reconfiguration");
  }
};

}

config_singleton::config_singleton()
  : component( std::make_shared<impl>() )
{
}

}}
