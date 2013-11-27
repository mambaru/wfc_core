

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
  conf.prefix = "./default_log";
  /*
  conf.config = log_writer_config("./default.config.log");
  conf.daemon = log_writer_config("./default.daemon.log");
  conf.common = log_writer_config("./default.common.log");
  conf.debug = log_writer_config("./default.debug.log");
  conf.trace = log_writer_config("./default.trace.log");
  */
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
  
  bool emptypath = !_config.prefix.empty() && _config.prefix!="disabled";

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
    std::cout << "idle f_fun daemon" << std::endl;
    return true;
  }
};

void logger_module::_reg_loggers()
{
  std::function<bool()> f = []()->bool{ std::cout << "idle daemon" << std::endl; return true;};
  //_global->idle.insert( f );
  //_global->idle.insert( _daemon_log->callback([]()->bool{ std::cout << "idle daemon" << std::endl; return true;}) );
  //_global->idle.insert( f );
  //_global->idle.insert( f_fun() );
  _global->idle.push_back( _daemon_log->callback(f_fun()) );
  
  //_global->idle.insert( _daemon_log->callback([]()->bool{ std::cout << "idle daemon" << std::endl; return true;}) );
  //_global->idle.insert( _config_log->callback([]()->bool{ std::cout << "idle config" << std::endl; return true;}) );
  
  if ( auto lr = _global->loggers.lock() )
  {
    //std::cout << "void logger_module::_reg_loggers() " <<  _daemon_log.get() << std::endl;
    lr->set("daemon", _daemon_log);
    lr->set("config", _config_log);
    lr->set("common", _common_log);
    lr->set("debug",  _debug_log );
    lr->set("trace",  _trace_log );
  }
}

void logger_module::_unreg_loggers()
{
  if ( auto lr = _global->loggers.lock() )
  {
    std::weak_ptr<ilogger> nptr;
    lr->set("daemon", nptr );
    lr->set("config", nptr );
    lr->set("common", nptr );
    lr->set("debug",  nptr );
    lr->set("trace",  nptr );
  }
}


void logger_module::create( std::weak_ptr<global> gl )
{
  _global = gl.lock();
  _config = logger_module_config();

  _create_single();
  _reg_loggers();
  /*
  std::shared_ptr<logger> _daemon_log = std::make_shared<logger>( logger_config() );
  _config_log = _daemon_log;
  _common_log = _daemon_log;
  _debug_log = _daemon_log;
  _trace_log = _daemon_log;
  */
  
}

void logger_module::configure(const std::string& confstr)
{
  // std::cout << "config_module::configure " << confstr << std::endl;
  logger_module_config_json::serializer()(_config, confstr.begin(), confstr.end());

  if ( _config.single )
    _create_single();
  else
    _create_multi();

  if ( _config.enabled )
    _reg_loggers();
  else
    _unreg_loggers();
  /*
  logger_config lconf;
  lconf.sylog = _config.sylog;
  lconf.stdout = _config.stdout;
  lconf.lifetime = _config.lifetime;
  if ( _config.single )
  {
    lconf.path = _config.prefix + ".log";
    _daemon_log = std::make_shared<logger>( lconf );
    _config_log = _daemon_log;
    _common_log = _daemon_log;
    _debug_log = _daemon_log;
    _trace_log = _daemon_log;
  }
  else
  {
    lconf.path = _config.prefix + ".daemon.log";
    _daemon_log = std::make_shared<logger>( lconf );

    lconf.path = _config.prefix + ".config.log";
    _config_log = std::make_shared<logger>( lconf );

    lconf.path = _config.prefix + ".common.log";
    _common_log = std::make_shared<logger>( lconf );

    lconf.path = _config.prefix + ".debug.log";
    _debug_log = std::make_shared<logger>( lconf );

    lconf.path = _config.prefix + ".trace.log";
    _trace_log = std::make_shared<logger>( lconf );
  }

  if ( auto lr = _global->loggers.lock() )
  {
    lr->set("daemon", _daemon_log);
    lr->set("config", _config_log);
    lr->set("common", _common_log);
    lr->set("debug",  _debug_log );
    lr->set("trace",  _trace_log );
  }
  */

  
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
