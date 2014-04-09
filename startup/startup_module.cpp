
#include "startup.hpp"
#include "startup_module.hpp"
#include "startup_config_json.hpp"
#include <wfc/core/global.hpp>

#include "build_info.h"

#include <iostream>
namespace wfc{

imodule::priority startup_module::startup_priority()  const
{
  return imodule::priority::none;  
}

imodule::priority startup_module::shutdown_priority() const
{
  return imodule::priority::none;
}

std::string startup_module::version() const
{
  return startup_build_info_string;
}

std::string startup_module::description() const
{
  return std::string("startup module");
}

std::string startup_module::generate(const std::string& /*type*/)  const
{
  std::string result;  
  startup_config conf;
  startup_config_json::serializer()(conf, std::back_inserter(result));
  return result;
}

bool startup_module::parse_config(const std::string& confstr)
{
  startup_config conf;
  startup_config_json::serializer()(conf, confstr.begin(), confstr.end());
  return true;
}

void startup_module::create( const std::string& /*name*/, std::weak_ptr<global> gl )
{
  _global = gl; // TODO: _global не нужен
  _startup = std::make_shared<startup_impl>(gl);
  if ( auto g = _global.lock() )
    g->startup = _startup;
 
  
  //!!! global::static_global = gl;
}

void startup_module::configure(const std::string& confstr)
{
  startup_config_json::serializer()(_config, confstr.begin(), confstr.end());
  _startup->configure(_config);
}

void startup_module::initialize()
{
  
}

void startup_module::start()
{
  
}

void startup_module::stop()
{
  
}

void startup_module::shutdown()
{
  
}

void startup_module::idle()
{
  
}

}
