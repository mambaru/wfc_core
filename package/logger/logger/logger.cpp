//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2013-2015
//
// Copyright: See COPYING file that comes with this distribution
//

#include "logger.hpp"
#include "writer.hpp"
#include "aux.hpp"
#include "writer_options.hpp"
#include <iow/logger/global_log.hpp>
#include <wfc/logger.hpp>
#include <wfc/memory.hpp>
#include <iostream>
#include <memory>

namespace wfc{ namespace core{

logger::~logger()
{
  this->unreg_loggers_();
}

logger::config_type logger::generate(const std::string&) 
{
  logger::config_type conf;
  conf.custom["log-name"]=writer_options();
  return conf;
}

void logger::start()
{
  _summary = 0;
  _starttime = aux::mkdate();
}

void logger::stop()
{
  std::lock_guard<mutex_type> lk(_mutex);
  this->unreg_loggers_();
  iow::init_log(nullptr);
  _writers.clear();
}

void logger::reconfigure()
{
  std::lock_guard<mutex_type> lk(_mutex);

  auto opt = this->options();
  _deny.clear();
  _deny.insert(opt.deny.begin(), opt.deny.end());

  if ( !::iow::log_status() )
  {
    std::weak_ptr<logger> wthis = this->shared_from_this();

    ::iow::log_writer logfun = this->wrap([wthis](  
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
      }
      return true;
    }, nullptr); // wrap
    ::iow::init_log(logfun);
  }
  else
  {
    for ( const auto& w : _writers  )
    {
      writer_options wopt = static_cast<writer_options>(opt);
      this->customize_(w.first, wopt);
      w.second->initialize(wopt);
    }
  }
}

void logger::perform_io(data_ptr d, io_id_t /*io_id*/, output_handler_t callback)
{

  std::stringstream ss;
  ss << d;
  std::string cmd;
  std::string result;
  ss >> cmd;
  if ( cmd=="exit" )
  {
    if (callback!=nullptr) callback(nullptr);
    return;
  }

  if ( cmd == "help" )
  {
    result = "help!!!";
  }
  else if ( cmd == "allow" )
  {
    std::string value;
    ss >> value;
    _deny.erase(value);
  }
  else if ( cmd == "deny" )
  {
    std::string value;
    ss >> value;
    _deny.insert(value);
  }
  else if ( cmd == "list" )
  {
    
  }
  else if ( cmd == "get" )
  {
    
  }
  else if ( cmd == "set" )
  {
    std::string name;
    ss >> name;
    auto itr = _writers.find(name);
    if ( itr==_writers.end() || itr->second==nullptr )
    {
      result = std::string("ERROR: logger '") + cmd + "' not found";
    }
    else
    {
      bool ready = true;
      auto pwrite = itr->second;
      auto opt = pwrite->options();
      std::sort(opt.deny.begin(), opt.deny.end());
      std::string field;
      ss >> field;
      if (field == "milliseconds")
      {
        bool value;
        ss >> std::boolalpha >> value;
        opt.milliseconds = value;
      }
      else if (field == "limit")
      {
        size_t value;
        ss >> value;
        opt.limit = value;
      }
      else if (field == "stdout")
      {
        std::string value;
        ss >> value;
        opt.stdout = value;
      }
      else if (field == "syslog")
      {
        std::string value;
        ss >> value;
        opt.syslog = value;
      }
      else if (field == "path")
      {
        std::string value;
        ss >> value;
        opt.path = value;
      }
      else if (field == "deny")
      {
        std::string value;
        ss >> value;
        auto itr = std::find(opt.deny.begin(), opt.deny.end(), value);
        if (itr==opt.deny.end())
          opt.deny.push_back(value);
      }
      else if (field == "allow")
      {
        std::string value;
        ss >> value;
        auto itr = std::find(opt.deny.begin(), opt.deny.end(), value);
        if (itr!=opt.deny.end())
          opt.deny.erase(itr);
      }
      else
      {
        ready = false;
        result = std::string("ERROR: unkown field: ") + field; 
      }

      if ( ready )
      {
        pwrite->initialize(opt);
        result="Done";
      }
    }
  }
  callback(std::make_unique<data_type>(result.begin(), result.end()) );
}


bool logger::is_deny_(const std::string& some) const
{
  return _deny.find(some) != _deny.end();
}

auto logger::get_or_create_(const std::string& name, const std::string& type) -> ilogger_ptr
{
  std::lock_guard<mutex_type> lk(_mutex);
  
  if ( is_deny_(name) || is_deny_(type) )
    return nullptr;
  
  auto itr = _writers.find(name);
  if ( itr != _writers.end() )
    return itr->second;
  
  return find_or_create_(name);
}

auto logger::find_or_create_(const std::string& name) -> ilogger_ptr
{
  if ( auto g = this->global() )
  {
    if ( auto l = g->registry.get<ilogger>("logger", name, true) )
    {
      return l;
    }
  }
  return create_(name);
}

auto logger::create_(const std::string& name) -> ilogger_ptr
{
  logger_config opt = this->options();
  writer_ptr pwriter = std::make_shared<writer>( /*_summary, _mutex*/ this->shared_from_this());
  writer_options wopt = static_cast<writer_options>(opt);
  
  this->customize_(name, wopt);

  pwriter->initialize(wopt);
  _writers[name] = pwriter;
  
  if ( auto g = this->global() )
  {
    g->registry.set("logger", name,  pwriter, true);
  }
  
  return pwriter;
}

void logger::customize_(const std::string& name, writer_options& wopt) const
{
  const logger_config& opt = this->options();

  auto itr = opt.custom.find(name);
  if ( itr != opt.custom.end() )
  {
    wopt = itr->second;
  }

  if ( wopt.path.empty() )
  {
    wopt.path = std::string("./") + this->global()->instance_name;
  }

  if (opt.single)
  {
    wopt.path = wopt.path+ ".log";
  }
  else
  {
    wopt.path = wopt.path + "-" + name + ".log";
  }
}

void logger::unreg_loggers_()
{
  if ( auto g = this->global() )
  {
    for ( const auto& m : _writers )
    {
      g->registry.erase("logger", m.first);
    }
  }
}

}}
