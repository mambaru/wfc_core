
#include "core.hpp"
#include "core_module.hpp"
#include "core_config_json.hpp"
#include <comet/core/global.hpp>

#include "build_info.h"

#include <iostream>
namespace mamba{ namespace comet{

imodule::priority core_module::startup_priority()  const
{
  return imodule::priority::core;  
}

imodule::priority core_module::shutdown_priority() const
{
  return imodule::priority::core;
}

std::string core_module::version() const
{
  return std::string(build_info);
}

std::string core_module::description() const
{
  return std::string("Модуль ядра");
}

std::string core_module::generate(const std::string& type)  const
{
  std::string result;  
  core_config conf;
  core_config_json::serializer()(conf, std::back_inserter(result));
  return result;
}

bool core_module::parse_config(const std::string& confstr)
{
  try
  {
    core_config conf;
    core_config_json::serializer()(conf, confstr.begin(), confstr.end());
  }
  catch(const json::json_error& e)
  {
    // В лог ошибку
    return false;
  }
  return true;
}

void core_module::create( std::weak_ptr<global> gl )
{
  _global = gl;
  _core = std::make_shared<core>();
  if ( auto g = _global.lock() )
    g->core = _core;
}

void core_module::configure(const std::string& confstr)
{
  std::cout << "core_module::configure " << confstr << std::endl;
  
  core_config_json::serializer()(_config, confstr.begin(), confstr.end());
}

void core_module::initialize()
{
  
}

void core_module::start()
{
  
}

void core_module::stop()
{
  
}

void core_module::idle()
{
  
}

}}
