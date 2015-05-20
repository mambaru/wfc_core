
#include "logger_object.hpp"
#include <wfc/module/singleton.hpp>
#include <wfc/module/instance.hpp>
#include <wfc/json.hpp>
#include "logger.hpp"
#include "logger_config_json.hpp"

namespace wfc{
  
JSON_NAME2(logger_object_name, "logger")

class logger_object_impl: public ::wfc::singleton<
  logger_object_name,
  wfc::instance<logger>,
  logger_config_json
>
{  
};

logger_object::logger_object()
  : object( std::make_shared<logger_object_impl>() )
{
}

}
