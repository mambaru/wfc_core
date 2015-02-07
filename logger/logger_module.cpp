

#include "logger_module.hpp"
#include "logger_module_config_json.hpp"
#include <wfc/core/global.hpp>
#include <iostream>
#include <memory>
#include "logger_build_info.h"
#include <wfc/logger.hpp>
#include "logger.hpp"

namespace wfc{

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
  return logger_build_info_string;
}

std::string logger_module::description() const
{
  return std::string("Log module");
}

std::string logger_module::generate(const std::string& /*type*/)  const
{
  logger_module_config conf;
  conf.prefix = "./default_log";
  std::string result;  
  logger_module_config_json::serializer()(conf, std::back_inserter(result));
  return result;
}

bool logger_module::parse_config(const std::string& confstr)
{
  logger_module_config conf;
  logger_module_config_json::serializer()(conf, confstr.begin(), confstr.end());
  return true;
}

void logger_module::_create_single()
{
  logger_config lconf;
  lconf.sylog = _config.sylog;
  lconf.stdout = _config.stdout;
  lconf.lifetime = _config.lifetime;
  
  if ( !_config.prefix.empty() && _config.prefix!="disabled")
    lconf.path = _config.prefix + ".log";

  _daemon_log = std::make_shared<logger>( lconf );
  _config_log = _daemon_log;
  _common_log = _daemon_log;
  _debug_log = _daemon_log;
  _trace_log = _daemon_log;

}
void logger_module::_create_multi()
{
  logger_config lconf;
  lconf.sylog = _config.sylog;
  lconf.stdout = _config.stdout;
  lconf.lifetime = _config.lifetime;
  
  bool emptypath = _config.prefix.empty() || _config.prefix=="disabled";

  if ( !emptypath ) lconf.path = _config.prefix + ".daemon.log";
  _daemon_log = std::make_shared<logger>( lconf );

  if ( !emptypath ) lconf.path = _config.prefix +  ".config.log";
  _config_log = std::make_shared<logger>( lconf );

  if ( !emptypath ) lconf.path = _config.prefix + ".common.log";
  _common_log = std::make_shared<logger>( lconf );

  if ( !emptypath ) lconf.path = _config.prefix + ".debug.log";
  _debug_log = std::make_shared<logger>( lconf );

  if ( !emptypath ) lconf.path = _config.prefix + ".trace.log";
  _trace_log = std::make_shared<logger>( lconf );
}

struct f_fun
{
  bool operator()() const
  {
    return true;
  }
};

void logger_module::_reg_loggers()
{
  std::function<bool()> f = []()->bool{ 
    // std::c1out << "idle daemon" << std::endl; 
    return true;
  };
  
  /*if ( auto lr = _global->loggers )
  {*/
    _global->registry.set("log-daemon", _daemon_log);
    _global->registry.set("log-config", _config_log);
    _global->registry.set("log-common", _common_log);
    _global->registry.set("log-debug",  _debug_log );
    _global->registry.set("log-trace",  _trace_log );
  //}
}

void logger_module::_unreg_loggers()
{
  /*if ( auto lr = _global->loggers )
  {*/
    std::shared_ptr<ilogger> nptr;
    _global->registry.set("log-daemon", nptr );
    _global->registry.set("log-config", nptr );
    _global->registry.set("log-common", nptr );
    _global->registry.set("log-debug",  nptr );
    _global->registry.set("log-trace",  nptr );
  //}
}


void logger_module::create( const std::string& /*name*/, std::shared_ptr<global> gl )
{
  _global = gl;
  _config = logger_module_config();

  _create_single();
  _reg_loggers();
}

void logger_module::configure(const std::string& confstr, const std::string&)
{
  logger_module_config_json::serializer()(_config, confstr.begin(), confstr.end());

  if ( _config.single )
    _create_single();
  else
    _create_multi();

  if ( _config.enabled )
    _reg_loggers();
  else
    _unreg_loggers();
}

void logger_module::initialize(const std::string&)
{
  
}

void logger_module::start(const std::string&)
{
  
}

void logger_module::stop(const std::string&)
{
}

void logger_module::shutdown(const std::string&)
{
  
}

void logger_module::idle()
{
  
}

}
