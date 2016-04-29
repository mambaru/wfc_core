
#include "config_singleton.hpp"
#include <wfc/module/singleton.hpp>
#include <wfc/module/instance.hpp>
#include <wfc/name.hpp>
#include "config.hpp"
#include "config_config_json.hpp"

namespace wfc{
  
WFC_NAME2(config_singleton_name, "config")

class config_singleton_impl: public ::wfc::singleton<
  config_singleton_name,
  wfc::instance<config>,
  config_config_json
>
{  
};

config_singleton::config_singleton()
  : component( std::make_shared<config_singleton_impl>() )
{
}

}
