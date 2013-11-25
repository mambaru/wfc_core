

#include "logger_module.hpp"
#include "logger_config_json.hpp"
#include <comet/core/global.hpp>
#include <iostream>
#include <memory>
#include "build_info.h"
#include <comet/logger.hpp>
#include "logger.hpp"

namespace mamba{ namespace comet{

imodule::priority logger_module::startup_priority()  const
{
  return imodule::priority::logger;  
}

imodule::priority logger_module::shutdown_priority() const
{
  return imodule::priority::logger;
}

std::string logger_module::version() const
{
  return std::string(build_info);
}

std::string logger_module::description() const
{
  return std::string("Модуль ядра");
}

std::string logger_module::generate(const std::string& type)  const
{
  std::string result;  
  logger_config conf;
  logger_config_json::serializer()(conf, std::back_inserter(result));
  return result;
}

bool logger_module::parse_config(const std::string& confstr)
{
  try
  {
    logger_config conf;
    logger_config_json::serializer()(conf, confstr.begin(), confstr.end());
  }
  catch(const json::json_error& e)
  {
    std::cout << "error " << e.message(confstr.begin(), confstr.end()) << std::endl;
    // В лог ошибку
    return false;
  }
  return true;
}

void logger_module::create( std::weak_ptr<global> gl )
{
  _global = gl.lock();
  _logger = std::make_shared<logger>(_global);
  _global->logger = _logger;
  global::global_logger = _logger;
}

void logger_module::configure(const std::string& confstr)
{
  // std::cout << "config_module::configure " << confstr << std::endl;
  logger_config_json::serializer()(_logger_config, confstr.begin(), confstr.end());
  _logger->configure(_logger_config);
  CONFIG_LOG_MESSAGE("logger_module: configured");
  CONFIG_LOG_BEGIN_PROCESS("logger_module: configured");
}

void logger_module::initialize()
{
  
}

void logger_module::start()
{
  
}

void logger_module::stop()
{
  
}

void logger_module::idle()
{
  
}

}}
