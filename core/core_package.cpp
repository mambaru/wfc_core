
#include "core_package.hpp"
#include "core_module.hpp"
#include <wfc/module/module_list.hpp>
#include <iow/json/name.hpp>

namespace wfc{
  
JSON_NAME2(core_package_name, "core")

class core_package_impl: public ::wfc::module_list<
  core_package_name,
  core_module
>
{  
};

core_package::core_package()
  : package( std::make_shared<core_package_impl>() )
{
}

}
