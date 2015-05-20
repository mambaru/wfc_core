
#include "logger_package.hpp"
#include "logger_module.hpp"
#include <wfc/module/module_list.hpp>
#include <iow/json/name.hpp>

namespace wfc{
  
JSON_NAME2(logger_package_name, "logger")

class logger_package_impl: public ::wfc::module_list<
  logger_package_name,
  logger_module
>
{  
};

logger_package::logger_package()
  : package( std::make_shared<logger_package_impl>() )
{
}

}
