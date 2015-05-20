
#include "core_module.hpp"
#include "core_object.hpp"
#include <wfc/module/object_list.hpp>
#include <wfc/json.hpp>

namespace wfc{
  
JSON_NAME2(core_module_name, "core")

class core_module_impl: public ::wfc::object_list<
  core_module_name,
  core_object
>
{  
};

core_module::core_module()
  : module( std::make_shared<core_module_impl>() )
{
}

}
