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


namespace wfc{

class logger_writer;

class logger_domain
  : public domain_object<iinterface, logger_config>
  , public std::enable_shared_from_this<logger_domain>
{
public:
  
  virtual void reconfigure();
  virtual void stop(const std::string& );
  virtual void start(const std::string& );
  
private:
  
  void create_single_();
  void create_multi_();
  void reg_loggers_();
  void unreg_loggers_();
  
private:
  
  std::shared_ptr<logger_writer> _config_log;
  std::shared_ptr<logger_writer> _domain_log;
  std::shared_ptr<logger_writer> _common_log;
  std::shared_ptr<logger_writer> _debug_log;
  std::shared_ptr<logger_writer> _syslog_log;
};

}
