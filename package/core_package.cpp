
#include "core_build_info.h"
#include "core_package.hpp"
#include "core/core_module.hpp"
#include "startup/startup_module.hpp"
#include "config/config_module.hpp"
#include "logger/logger_module.hpp"
#include "workflow/workflow_module.hpp"
#include <wfc/module/module_list.hpp>

namespace wfc{
  
namespace 
{
  class impl: public ::wfc::module_list<
    core_build_info,
      workflow_module,
      logger_module,
      config_module,
      core_module,
      startup_module
  >
  {};
}

core_package::core_package()
  : package( std::make_shared<impl>() )
{
}

}
