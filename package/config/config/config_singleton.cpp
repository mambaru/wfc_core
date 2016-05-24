
#include "config_singleton.hpp"
#include <wfc/module/singleton.hpp>
#include <wfc/module/instance.hpp>
#include <wfc/name.hpp>
#include "config.hpp"
#include "config_config_json.hpp"

namespace wfc{
  
namespace{

WFC_NAME2(config_singleton_name, "config")

class impl: public ::wfc::singleton<
  config_singleton_name,
  wfc::instance<config>,
  config_config_json,
    component_features::DisableSuspend
  | component_features::CommonWorkflow
  | component_features::Extraordinary
>
{  
};

}

config_singleton::config_singleton()
  : component( std::make_shared<impl>() )
{
}

}
