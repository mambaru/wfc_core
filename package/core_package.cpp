
#include "core_build_info.h"
#include "core_package.hpp"
#include "core/core_module.hpp"
#include <wfc/module/module_list.hpp>
#include <iow/json/name.hpp>

namespace wfc{
  
class core_package_impl: public ::wfc::module_list<
  core_build_info,
  core_module
>
{  
};

core_package::core_package()
  : package( std::make_shared<core_package_impl>() )
{
}

}
