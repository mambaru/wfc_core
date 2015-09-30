//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2013-2015
//
// Copyright: See COPYING file that comes with this distribution
//

#pragma once

#include <wfc/domain_object.hpp>
#include <wfc/logger/ilogger.hpp>
#include "logger_config.hpp"
#include <memory>
#include <string>
#include <set>

namespace wfc{

class logger_writer;

class logger_domain
  : public domain_object<iinterface, logger_config>
  , public std::enable_shared_from_this<logger_domain>
{
public:
  typedef std::shared_ptr<ilogger> ilogger_ptr;
  typedef std::shared_ptr<logger_writer> writer_ptr;
  typedef std::map<std::string, writer_ptr> writer_map;


  virtual void reconfigure();
  virtual void stop(const std::string& );
  virtual void start(const std::string& );
  
private:
  bool is_deny_(const std::string& some) const;
  ilogger_ptr get_or_create_(const std::string& name, const std::string& type);
  ilogger_ptr find_or_create_(const std::string& name);
  ilogger_ptr create_(const std::string& name);
  void customize_(const std::string& name, writer_config& opt) const;
  void unreg_loggers_();
  
  /*
  void create_single_();
  void create_multi_();
  void reg_loggers_();
  
  void reg_log_(std::string name, writer_ptr writer);
  */

private:
  std::set<std::string> _deny;
  writer_map _writers;

  /*
  writer_ptr _config_log;
  writer_ptr _domain_log;
  writer_ptr _common_log;
  writer_ptr _debug_log;
  writer_ptr _jsonrpc_log;
  writer_ptr _iow_log;
  writer_ptr _syslog_log;
  */
};

}
