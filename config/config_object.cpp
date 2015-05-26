
#include "config_object.hpp"
#include <wfc/module/singleton.hpp>
#include <wfc/module/instance.hpp>
#include <wfc/name.hpp>
#include "config.hpp"
#include "config_config_json.hpp"

namespace wfc{
  
WFC_NAME2(config_object_name, "config")

class config_object_impl: public ::wfc::singleton<
  config_object_name,
  wfc::instance<config>,
  config_config_json
>
{  
};

config_object::config_object()
  : object( std::make_shared<config_object_impl>() )
{
}

}
