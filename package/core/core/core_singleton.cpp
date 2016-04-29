
#include "core_singleton.hpp"
#include <wfc/module/singleton.hpp>
#include <wfc/module/instance.hpp>
#include <wfc/json.hpp>
#include "core.hpp"
#include "core_config_json.hpp"

namespace wfc{

namespace{
JSON_NAME2(core_singleton_name, "core")

class impl: public ::wfc::singleton<
  core_singleton_name,
  wfc::instance<core>,
  core_config_json,
  int(component_features::Fixed) | int(component_features::Extraordinary)
>
{  
};
}

core_singleton::core_singleton()
  : component( std::make_shared<impl>() )
{
}

}
