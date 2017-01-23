//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2013-2015
//
// Copyright: See COPYING file that comes with this distribution
//

#pragma once

#include "writer_options.hpp"
#include "logger_config.hpp"

#include <wfc/domain_object.hpp>
#include <wfc/logger/ilogger.hpp>

#include <memory>
#include <mutex>
#include <string>
#include <set>

namespace wfc{ namespace core{

class writer;

class logger
  : public domain_object<iinterface, logger_config, nostat>
  , public std::enable_shared_from_this<logger>
{
  typedef std::mutex mutex_type;
  friend class writer;

public:
  typedef std::shared_ptr<ilogger> ilogger_ptr;
  typedef std::shared_ptr<writer> writer_ptr;
  typedef std::map<std::string, writer_ptr> writer_map;

  virtual ~logger();
  
// domain_object
  virtual void reconfigure() override;
  virtual void stop() override;
  virtual void start() override;
  virtual logger::config_type generate(const std::string&) override;
//iinterface
  virtual void perform_io(data_ptr d, io_id_t io_id, outgoing_handler_t callback);
  
private:
  ilogger_ptr get_or_create_(const std::string& name, const std::string& type);
  bool is_deny_(const std::string& some) const;
  ilogger_ptr find_or_create_(const std::string& name);
  ilogger_ptr create_(const std::string& name);
  void customize_(const std::string& name, writer_options& opt) const;
  void unreg_loggers_();

private:
  std::set<std::string> _deny;
  writer_map _writers;
  size_t _summary;
  std::string _starttime;
  mutable mutex_type _mutex;
};

}}
