//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2013-2015
//
// Copyright: See COPYING file that comes with this distribution
//

#pragma once

#include "logger_config.hpp"

#include <wfc/domain_object.hpp>
#include <wfc/logger/ilogger.hpp>
#include <wlog/logger/logger_handlers.hpp>

#include <memory>
#include <mutex>
#include <string>
#include <set>

namespace wfc{ namespace core{

class logger
  : public domain_object<iinterface, logger_config, nostat>
  , public std::enable_shared_from_this<logger>
{
  typedef std::mutex mutex_type;
public:
  typedef domain_object::domain_config domain_config;
  
  virtual ~logger();
  logger();
// domain_object
  virtual void initialize() override;
  virtual void reconfigure() override;
  virtual domain_config generate(const std::string&) override;
//iinterface
  virtual void perform_io(data_ptr d, io_id_t io_id, output_handler_t callback) override;
private:
  void init_log_(wlog::logger_options opt, wlog::logger_handlers dlh);
  virtual void release();
  typedef std::tuple<wlog::time_point, std::string, std::string, std::string> message_t;
  std::unique_ptr<message_t> _last_message;
  mutable mutex_type _mutex;
};

}}
