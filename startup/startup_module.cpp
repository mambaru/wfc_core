
#include "startup_module.hpp"
#include "startup_object.hpp"
#include <wfc/module/object_list.hpp>
#include <wfc/json.hpp>

namespace wfc{
  
JSON_NAME2(startup_module_name, "startup")

class startup_module_impl: public ::wfc::object_list<
  startup_module_name,
  startup_object
>
{  
};

startup_module::startup_module()
  : module( std::make_shared<startup_module_impl>() )
{
}

}

/*
#include "startup.hpp"
#include "startup_module.hpp"
#include "startup_config_json.hpp"
#include <wfc/core/global.hpp>

#include "startup_build_info.h"

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

std::string startup_module::generate(const std::string&)  const
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

void startup_module::create( const std::string& , std::shared_ptr<global> gl )
{
  _global = gl; // TODO: _global не нужен
  _startup = std::make_shared<startup_impl>(gl);
  if ( auto g = _global.lock() )
  {
    g->registry.set("startup", _startup);
    //g->startup = _startup;
  }
 
  
  //!!! global::static_global = gl;
}

void startup_module::configure(const std::string& confstr, const std::string&)
{
  startup_config_json::serializer()(_config, confstr.begin(), confstr.end());
  _startup->configure(_config);
}

void startup_module::initialize(const std::string&)
{
  
}

void startup_module::start(const std::string&)
{
  
}

void startup_module::stop(const std::string&)
{
  
}

void startup_module::shutdown(const std::string&)
{
  
}

void startup_module::idle()
{
  
}

}
*/
