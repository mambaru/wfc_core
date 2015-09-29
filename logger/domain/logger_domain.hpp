//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2013-2015
//
// Copyright: See COPYING file that comes with this distribution
//

#pragma once

#include <wfc/domain_object.hpp>
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
  typedef std::shared_ptr<logger_writer> writer_ptr;
  
public:

  virtual void reconfigure();
  virtual void stop(const std::string& );
  virtual void start(const std::string& );
  
private:
  
  void create_single_();
  void create_multi_();
  void reg_loggers_();
  void unreg_loggers_();
  void reg_log_(std::string name, writer_ptr writer);

private:
  std::set<std::string> _reject;

  writer_ptr _config_log;
  writer_ptr _domain_log;
  writer_ptr _common_log;
  writer_ptr _debug_log;
  writer_ptr _jsonrpc_log;
  writer_ptr _iow_log;
  writer_ptr _syslog_log;
};

}
