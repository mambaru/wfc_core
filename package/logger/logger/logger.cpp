//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2013-2017
//
// Copyright: See COPYING file that comes with this distribution
//

#include "logger.hpp"
#include <wfc/logger.hpp>
#include <wfc/wfc_exit.hpp>
#include <wfc/memory.hpp>
#include <wlog/init_log.hpp>
#include <wlog/formatter/formatter.hpp>
#include <iostream>
#include <memory>

namespace wfc{ namespace core{

logger::~logger()
{
  // Логгер инициализируеться на этапе конфигурации
  // Но если система не была запущщена из=за ошибки, 
  // то метод stop не будет вызван
  this->release();
}

logger::logger()
{
  wlog::init_log();
}

logger::config_type logger::generate(const std::string& arg) 
{
  domain_object::config_type  conf;
  if ( arg == "example" )
  {
    conf.startup_priority = -1000;
    conf.shutdown_priority = 1000;
    conf.customize.resize(1);
    conf.customize.back().names.push_back("<<log-name>>");
    conf.customize.back().deny.insert("DEBUG");
    conf.customize.back().path="$";
    conf.customize.back().resolution = wlog::resolutions::milliseconds;
    conf.customize.back().size_limit = 12345;
    conf.deny.insert("TRACE");
    conf.path="path/to/log/file.log";
  }
  
  conf.finalize();
  return conf;
}

void logger::release()
{
  std::unique_ptr<message_t> lm;
  wlog::logger_fun log;
  {
    std::lock_guard<mutex_type> lk(_mutex);
    log = wlog::release_log();
    if ( _last_message!=nullptr)
      lm = std::make_unique<message_t>( *_last_message );
  }
  // TODO: записать финальное сообщение 
  if (lm != nullptr && log!=nullptr )
  {
    auto tp = wlog::time_point::clock::now();
    log(tp, "", "FINAL", "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
    
    {
      std::stringstream ss;
      ss << "Date: ";
      wlog::formatter::date(ss, std::get<0>( *lm), this->options(), wlog::formatter_handlers() );
      ss << "\n";
      log(tp, "", "FINAL", ss.str() );
    }

    {
      std::stringstream ss;
      ss << "Time: ";
      wlog::formatter::time(ss, std::get<0>( *lm), this->options(), wlog::formatter_handlers() );
      wlog::formatter::fraction(ss, std::get<0>( *lm), this->options(), wlog::formatter_handlers() );
      ss << "\n";
      log(tp, "", "FINAL", ss.str() );
    }

    {
      std::stringstream ss;
      ss << "Name: ";
      ss << std::get<1>( *lm);
      ss << "\n";
      log(tp, "", "FINAL", ss.str() );
    }

    {
      std::stringstream ss;
      ss << "Message: ";
      ss << std::get<3>( *lm);
      log(tp, "", "FINAL", ss.str() );
    }
    log(tp, "", "FINAL", "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
  }
}

void logger::reconfigure()
{
  this->init_log_( wlog::logger_handlers() );
}

void logger::init_log_(wlog::logger_handlers dlh)
{
  auto opt = this->options();
  bool fatal_found = false;
  bool final_found = false;
  bool syslog_found = false;
  for ( const auto& c : opt.customize )
  {
    for ( const auto& n : c.names)
    {
      if ( !fatal_found && n == "FATAL")
        fatal_found = true;
      if ( !final_found && n == "FINAL")
        final_found = true;
      if ( !syslog_found && n == "SYSLOG")
        syslog_found = true;
    }
  }
  
  if ( !fatal_found )
  {
    opt.customize.resize( opt.customize.size() + 1 );
    opt.customize.back().names.push_back("FATAL");
    opt.customize.back().stdout.color_map["$all"] = "red";
  }
    
  if ( !final_found )
  {
    opt.customize.resize( opt.customize.size() + 1 );
    opt.customize.back().names.push_back("FINAL");
    opt.customize.back().stdout.color_map["$all"] = "light_red";
  }
  
  if ( !syslog_found )
  {
    opt.customize.resize( opt.customize.size() + 1 );
    opt.customize.back().names.push_back("SYSLOG");
    opt.customize.back().syslog.name=this->global()->program_name;
  }
  
  bool stop_by_fatal = opt.stop_with_fatal_log_entry;
  dlh.after.push_back([this, stop_by_fatal](const wlog::time_point& tp, const std::string& name, const std::string& ident, const std::string& message)
  {
    if ( ident=="FATAL" )
    {
      if ( auto g = this->global())
      {
        {
          std::lock_guard<mutex_type> lk( this->_mutex);
          if ( _last_message == nullptr)
            _last_message = std::make_unique<message_t>(tp, name, ident, message);
        }

        if ( stop_by_fatal && g->stop_signal_flag == false )
        {
          wfc_exit_with_error("Abnormal Shutdown by 'FATAL' message!");
        }
      }
    }
  });
  
  wlog::init_log( opt, dlh );
}

void logger::initialize()
{
  bool has_handlers = false;
  auto logs= this->select_targets<ilogger>("logger");
  has_handlers |= !logs.empty();
  if ( has_handlers )
  {
    wlog::logger_handlers dlh;
    for ( const auto& fmt: logs )
    {
      dlh.customize[fmt.first].file_formatter   = fmt.second->formatter();
      dlh.customize[fmt.first].stdout_formatter = fmt.second->formatter();
      dlh.customize[fmt.first].syslog_formatter = fmt.second->formatter();
    }
    this->init_log_(dlh);
  }
}

void logger::perform_io(data_ptr /*d*/, io_id_t /*io_id*/, output_handler_t /*callback*/)
{
  /*

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
  */
}


}}
