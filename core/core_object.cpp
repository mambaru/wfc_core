
#include "core_object.hpp"
#include <wfc/module/singleton.hpp>
#include <wfc/module/instance.hpp>
#include <wfc/json.hpp>
#include "core.hpp"
#include "core_config_json.hpp"

namespace wfc{
  
JSON_NAME2(core_object_name, "core")

class core_object_impl: public ::wfc::singleton<
  core_object_name,
  wfc::instance<core>,
  core_config_json
>
{  
};

core_object::core_object()
  : object( std::make_shared<core_object_impl>() )
{
}

}
