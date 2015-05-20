#include "logger.hpp"
#include <iostream>
#include <memory>
#include "logger_build_info.h"
#include <wfc/logger.hpp>
#include "logger_writer.hpp"
#include "writer_config.hpp"


#include <iow/logger/global_log.hpp>

namespace wfc{

void logger::start(const std::string& )
{
  std::cout << "DEBUG LOGGER START!!!" << std::endl;
  if ( !::iow::log_status() )
  {
    std::weak_ptr<logger> wthis = this->shared_from_this();
    
    ::iow::log_writer logfun = [wthis](  
        const std::string& name, 
        const std::string& type, 
        const std::string& str) -> bool
    {
      if ( auto pthis = wthis.lock() )
      {
        if ( auto g = pthis->global() )
        {
          if ( auto l = g->registry.get<ilogger>("logger", name) )
          {
            l->write(name, type, str);
            return true;
          }
          else
          {
            std::cerr << "LOGGER ERROR: logger '" << name << "' not found" << std::endl;
          }
        }
      }
      return false;
    };
    
    ::iow::init_log(logfun);
  }
}

void logger::stop(const std::string& )
{
  this->unreg_loggers_();
  iow::init_log(nullptr);
}

void logger::reconfigure()
{
  auto opt = this->options();
  opt.single
    ? this->create_single_()
    : this->create_multi_();
  this->reg_loggers_();
}

void logger::create_single_()
{
  auto conf = this->options();
  writer_config wconf;
  wconf.sylog = conf.sylog;
  wconf.stdout = conf.stdout;
  wconf.lifetime = conf.lifetime;
  
  if ( !conf.prefix.empty() && conf.prefix!="disabled")
    wconf.path = conf.prefix + ".log";

  _domain_log = std::make_shared<logger_writer>();
  _domain_log->initialize(wconf);
  _config_log = _domain_log;
  _common_log = _domain_log;
  _debug_log = _domain_log;
}

void logger::create_multi_()
{
  auto conf = this->options();
  writer_config wconf;
  wconf.sylog = conf.sylog;
  wconf.stdout = conf.stdout;
  wconf.lifetime = conf.lifetime;
  
  bool emptypath = conf.prefix.empty() || conf.prefix=="disabled";

  if ( !emptypath ) wconf.path = conf.prefix + ".domain.log";
  _domain_log = std::make_shared<logger_writer>();
  _domain_log->initialize(wconf);

  if ( !emptypath ) wconf.path = conf.prefix +  ".config.log";
  _config_log = std::make_shared<logger_writer>();
  _config_log->initialize(wconf);

  if ( !emptypath ) wconf.path = conf.prefix + ".common.log";
  _common_log = std::make_shared<logger_writer>();
  _common_log->initialize(wconf);

  if ( !emptypath ) wconf.path = conf.prefix + ".debug.log";
  _debug_log = std::make_shared<logger_writer>();
  _debug_log->initialize(wconf);

}

void logger::reg_loggers_()
{
  if ( auto g = this->global() )
  {
    g->registry.set("logger", "domain", _domain_log);
    g->registry.set("logger", "config", _config_log);
    g->registry.set("logger", "common", _common_log);
    g->registry.set("logger", "debug",  _debug_log );
  }
}

void logger::unreg_loggers_()
{
  if ( auto g = this->global() )
  {
    g->registry.erase("logger", "domain");
    g->registry.erase("logger", "config");
    g->registry.erase("logger", "common");
    g->registry.erase("logger", "debug");
  }
}

}
