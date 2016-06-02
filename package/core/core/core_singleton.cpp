#include <wfc/module/singleton.hpp>
#include <wfc/module/instance.hpp>
#include <wfc/name.hpp>

#include "core_singleton.hpp"
#include "core.hpp"
#include "core_config_json.hpp"

namespace wfc{

namespace
{
  WFC_NAME2(singleton_name, "core")

  class impl: public ::wfc::singleton
  <
    singleton_name,
    wfc::instance<core>,
    core_config_json,
    component_features::DisabledEnabled 
    | component_features::DisabledPriority
    | component_features::DisabledSuspend
    | component_features::DisabledWorkflow 
  >
  {};

}

core_singleton::core_singleton()
  : component( std::make_shared<impl>() )
{
}

}
