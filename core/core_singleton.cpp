
#include "core_singleton.hpp"
#include <wfc/module/singleton.hpp>
#include <wfc/module/instance.hpp>
#include <wfc/json.hpp>
#include "core.hpp"
#include "core_config_json.hpp"

namespace wfc{

JSON_NAME2(core_singleton_name, "core")

class core_singleton_impl: public ::wfc::singleton<
  core_singleton_name,
  wfc::instance<core>,
  core_config_json
>
{  
};

core_singleton::core_singleton()
  : component( std::make_shared<core_singleton_impl>() )
{
}

}
