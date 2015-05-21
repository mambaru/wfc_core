//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2013-2015
//
// Copyright: See COPYING file that comes with this distribution
//

#include "logger_domain.hpp"
#include "logger_writer.hpp"
#include "writer_config.hpp"
#include <iow/logger/global_log.hpp>
#include <wfc/logger.hpp>
#include <iostream>
#include <memory>


namespace wfc{

void logger_domain::start(const std::string& )
{
}

void logger_domain::stop(const std::string& )
{
  this->unreg_loggers_();
  iow::init_log(nullptr);
}

void logger_domain::reconfigure()
{
  auto opt = this->options();
  opt.single
    ? this->create_single_()
    : this->create_multi_();
  this->reg_loggers_();
  
  // TODO: Перенести в create
  if ( !::iow::log_status() )
  {
    std::weak_ptr<logger_domain> wthis = this->shared_from_this();
    
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

void logger_domain::create_single_()
{
  writer_config wconf = static_cast<writer_config>( this->options() );
  if ( !wconf.path.empty() && wconf.path!="disabled")
    wconf.path = wconf.path+ ".log";

  _domain_log = std::make_shared<logger_writer>();
  _domain_log->initialize(wconf);
  _config_log = _domain_log;
  _common_log = _domain_log;
  _debug_log  = _domain_log;
  _syslog_log = _domain_log;
}

void logger_domain::create_multi_()
{
  writer_config wconf = static_cast<writer_config>( this->options() );
  
  std::string path = wconf.path;
  bool emptypath = path.empty() || path=="disabled";

  if ( !emptypath ) wconf.path = path + ".domain.log";
  _domain_log = std::make_shared<logger_writer>();
  _domain_log->initialize(wconf);

  if ( !emptypath ) wconf.path = path +  ".config.log";
  _config_log = std::make_shared<logger_writer>();
  _config_log->initialize(wconf);

  if ( !emptypath ) wconf.path = path + ".common.log";
  _common_log = std::make_shared<logger_writer>();
  _common_log->initialize(wconf);

  if ( !emptypath ) wconf.path = path + ".debug.log";
  _debug_log = std::make_shared<logger_writer>();
  _debug_log->initialize(wconf);

   if ( !emptypath ) wconf.path = path + ".syslog.log";
  _syslog_log = std::make_shared<logger_writer>();
  _syslog_log->initialize(wconf);
}

void logger_domain::reg_loggers_()
{
  if ( auto g = this->global() )
  {
    g->registry.set("logger", "domain", _domain_log);
    g->registry.set("logger", "config", _config_log);
    g->registry.set("logger", "common", _common_log);
    g->registry.set("logger", "debug",  _debug_log );
    g->registry.set("logger", "syslog", _syslog_log );
  }
}

void logger_domain::unreg_loggers_()
{
  if ( auto g = this->global() )
  {
    g->registry.erase("logger", "domain");
    g->registry.erase("logger", "config");
    g->registry.erase("logger", "common");
    g->registry.erase("logger", "debug");
    g->registry.erase("logger", "syslog");
  }
}

}
