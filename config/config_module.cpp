
#include "config.hpp"
#include "config_module.hpp"
#include "config_config_json.hpp"
#include <wfc/core/global.hpp>
#include <iostream>
#include "build_info.h"

namespace wfc{

imodule::priority config_module::startup_priority()  const
{
  return imodule::priority::config;  
}

imodule::priority config_module::shutdown_priority() const
{
  return imodule::priority::config;
}

std::string config_module::version() const
{
  return config_build_info_string;
}

std::string config_module::description() const
{
  return std::string("Модуль ядра");
}

std::string config_module::generate(const std::string& type)  const
{
  std::string result;  
  config_config conf;
  config_config_json::serializer()(conf, std::back_inserter(result));
  return result;
}

bool config_module::parse_config(const std::string& confstr)
{
  config_config conf;
  config_config_json::serializer()(conf, confstr.begin(), confstr.end());
  return true;
}

void config_module::create( const std::string& name, std::weak_ptr<global> gl )
{
  _global = gl.lock();
  _config = std::make_shared<config>(_global);
  _global->config = _config;
}

void config_module::configure(const std::string& confstr)
{
  config_config_json::serializer()(_config_config, confstr.begin(), confstr.end());
  _config->configure(_config_config);
}

void config_module::initialize()
{
  
}

void config_module::start()
{
  
}

void config_module::stop()
{
  
}

void config_module::idle()
{
  
}

}
