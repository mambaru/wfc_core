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

  _deny.clear();
  _deny.insert(opt.deny.begin(), opt.deny.end());

  /*
  opt.single
    ? this->create_single_()
    : this->create_multi_();

  this->reg_loggers_();
  */

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
        if (auto log = pthis->get_or_create_( name, type) )
        {
          log->write(name, type, str);
          return true;
        }
        /*
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
        */
      }
      return false;
    };
    ::iow::init_log(logfun);
  }
}


bool logger_domain::is_deny_(const std::string& some) const
{
  return _deny.find(some) != _deny.end();
}

auto logger_domain::get_or_create_(const std::string& name, const std::string& type) -> ilogger_ptr
{
  if ( is_deny_(name) || is_deny_(type) )
    return nullptr;
  
  auto itr = _writers.find(name);
  if ( itr != _writers.end() )
    return itr->second;
  
  return find_or_create_(name);
}

auto logger_domain::find_or_create_(const std::string& name) -> ilogger_ptr
{
  if ( auto g = this->global() )
  {
    if ( auto l = g->registry.get<ilogger>("logger", name) )
    {
      return l;
    }
  }
  return create_(name);
}


auto logger_domain::create_(const std::string& name) -> ilogger_ptr
{
  logger_config opt = this->options();
  writer_ptr pwriter = std::make_shared<logger_writer>();
  writer_config wopt = static_cast<writer_config>(opt);
  if (opt.single)
  {
    wopt.path = wopt.path+ ".log";
  }
  else
  {
    wopt.path = wopt.path + "-" + name+ ".log";
  }
  
  this->customize_(name, wopt);
  /*
  auto itr = opt.custom.find(name);
  if ( itr != opt.custom.end() )
  {
    writer_config cstm= itr->second;
    if (cstm.limit!=0)
      wopt.limit = cstm.limit;
    if (!cstm.path.empty())
      wopt.path = cstm.path;
    if (!cstm.syslog.empty())
      wopt.syslog = cstm.syslog;
    wopt.stdout = cstm.stdout;
    wopt.deny = cstm.deny;
  }
  */

  pwriter->initialize(wopt);
  _writers[name] = pwriter;
  
  if ( auto g = this->global() )
  {
    g->registry.set("logger", name,  pwriter);
  }
  
  return pwriter;
}

void logger_domain::customize_(const std::string& name, writer_config& wopt) const
{
  const logger_config& opt = this->options();

  auto itr = opt.custom.find(name);
  if ( itr != opt.custom.end() )
  {
    writer_config cstm= itr->second;
    if (cstm.limit!=0)
      wopt.limit = cstm.limit;
    if (!cstm.path.empty())
      wopt.path = cstm.path;
    
    if (!cstm.syslog.empty())
      wopt.syslog = cstm.syslog;
    wopt.stdout = cstm.stdout;
    wopt.deny = cstm.deny;
  }
}

void logger_domain::unreg_loggers_()
{
  if ( auto g = this->global() )
  {
    for ( const auto& m : _writers )
    {
      g->registry.erase("logger", m.first);
    }
  }
}


/*

void logger_domain::create_single_()
{
  writer_config wconf = static_cast<writer_config>( this->options() );
  if ( !wconf.path.empty() && wconf.path!="disabled")
    wconf.path = wconf.path+ ".log";

  _domain_log = std::make_shared<logger_writer>();
  _domain_log->initialize(wconf);
  _config_log  = _domain_log;
  _common_log  = _domain_log;
  _debug_log   = _domain_log;
  _jsonrpc_log = _domain_log;
  _iow_log     = _domain_log;
  _syslog_log  = _domain_log;
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

  if ( !emptypath ) wconf.path = path + ".jsonrpc.log";
  _jsonrpc_log = std::make_shared<logger_writer>();
  _jsonrpc_log->initialize(wconf);

  if ( !emptypath ) wconf.path = path + ".iow.log";
  _iow_log = std::make_shared<logger_writer>();
  _iow_log->initialize(wconf);

   if ( !emptypath ) wconf.path = path + ".syslog.log";
  _syslog_log = std::make_shared<logger_writer>();
  _syslog_log->initialize(wconf);
}



void logger_domain::reg_log_(std::string name, writer_ptr writer)
{
  if ( auto g = this->global() )
  {
    if ( _deny.count(name) == 0)
    {
      g->registry.set("logger", name,  writer);
    }
  }
}

void logger_domain::reg_loggers_()
{
  this->reg_log_("domain",   _domain_log);
  this->reg_log_("config",   _config_log);
  this->reg_log_("common",   _common_log);
  this->reg_log_("debug",    _debug_log );
  this->reg_log_("jsonrpc",  _jsonrpc_log );
  this->reg_log_("iow",      _iow_log );
  this->reg_log_("syslog",   _syslog_log );
}

void logger_domain::unreg_loggers_()
{
  if ( auto g = this->global() )
  {
    g->registry.erase("logger", "domain");
    g->registry.erase("logger", "config");
    g->registry.erase("logger", "common");
    g->registry.erase("logger", "debug");
    g->registry.erase("logger", "jsonrpc");
    g->registry.erase("logger", "iow");
    g->registry.erase("logger", "syslog");
  }
}
*/

}
