
#include "config.hpp"
#include "config_module.hpp"
#include "config_config_json.hpp"
#include <comet/core/global.hpp>
#include <iostream>
#include "build_info.h"

namespace mamba{ namespace comet{

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
  return std::string(build_info);
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
  try
  {
    config_config conf;
    config_config_json::serializer()(conf, confstr.begin(), confstr.end());
  }
  catch(const json::json_error& e)
  {
    std::cout << "error " << e.message(confstr.begin(), confstr.end()) << std::endl;
    // В лог ошибку
    return false;
  }
  return true;
}

void config_module::create( std::weak_ptr<global> gl )
{
  _global = gl.lock();
  _config = std::make_shared<config>(_global);
  _global->config = _config;
}

void config_module::configure(const std::string& confstr)
{
  std::cout << "config_module::configure " << confstr << std::endl;
  config_config_json::serializer()(_config_config, confstr.begin(), confstr.end());
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

}}
