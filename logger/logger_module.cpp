

#include "logger_module.hpp"
#include "logger_module_config_json.hpp"
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
  return std::string(logger_build_info);
}

std::string logger_module::description() const
{
  return std::string("Модуль ядра");
}

std::string logger_module::generate(const std::string& type)  const
{
  logger_module_config conf;
  /*
  conf.config = log_writer_config("./default.config.log");
  conf.daemon = log_writer_config("./default.daemon.log");
  conf.common = log_writer_config("./default.common.log");
  conf.debug = log_writer_config("./default.debug.log");
  conf.trace = log_writer_config("./default.trace.log");
  */
  std::string result;  
  // logger_config_json::serializer()(conf, std::back_inserter(result));
  return result;
}

bool logger_module::parse_config(const std::string& confstr)
{
  logger_module_config conf;
  logger_module_config_json::serializer()(conf, confstr.begin(), confstr.end());
  return true;
}

void logger_module::create( std::weak_ptr<global> gl )
{
  std::cout <<  "---void logger_module::create( std::weak_ptr<global> gl )---" << std::endl;

  _global = gl.lock();
  _config = logger_module_config();

  std::shared_ptr<logger> _daemon_log = std::make_shared<logger>( logger_config() );
  _config_log = _daemon_log;
  _common_log = _daemon_log;
  _debug_log = _daemon_log;
  _trace_log = _daemon_log;
  
  if ( auto lr = _global->loggers.lock() )
  {
    std::cout << "log insert" << std::endl;
    lr->insert("daemon", _daemon_log);
    lr->insert("config", _config_log);
    lr->insert("common", _common_log);
    lr->insert("debug",  _debug_log );
    lr->insert("trace",  _trace_log );
  }

  /*
    std::shared_ptr<logger> _config_log;
  std::shared_ptr<logger> _common_log;
  std::shared_ptr<logger> _debug_log;
  std::shared_ptr<logger> _trace_log;
*/
  /// TODO: по дефолту 5 логерров
  // _logger = std::make_shared<logger>();
  //!!! _global->logger = _logger;
  // _logger->configure( logger_config() );
}

void logger_module::configure(const std::string& confstr)
{
  // std::cout << "config_module::configure " << confstr << std::endl;
  logger_module_config_json::serializer()(_config, confstr.begin(), confstr.end());
  /// !!!_logger->configure(_logger_config);
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
