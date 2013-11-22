
#include "config.hpp"
#include "config_module.hpp"
#include "config_config_json.hpp"
#include <comet/core/global.hpp>

#include "build_info.h"

namespace mamba{ namespace comet{

imodule::priority config_module::startup_priority()  const
{
  return imodule::priority::core;  
}

imodule::priority config_module::shutdown_priority() const
{
  return imodule::priority::core;
}

std::string config_module::version() const
{
  return std::string(build_info);
}

std::string config_module::description() const
{
  return std::string("Модуль ядра");
}

std::string config_module::generate(const std::string& type)  const
{
  std::string result;  
  core_config conf;
  core_config_json::serializer()(conf, std::back_inserter(result));
  return result;
}

bool core_module::config_config(const std::string& confstr)
{
  try
  {
    config_config conf;
    config_config_json::serializer()(conf, confstr.begin(), confstr.end());
  }
  catch(const json::json_error& e)
  {
    // В лог ошибку
    return false;
  }
  return true;
}

void config_module::create( std::weak_ptr<global> gl )
{
  _global = gl;
  _config = std::make_shared<config>();
  if ( auto g = _global.lock() )
    g->config = _config;
}

void core_module::configure(const std::string& confstr)
{
  config_config_json::serializer()(_config_config, confstr.begin(), confstr.end());
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
