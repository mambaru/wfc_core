//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2013-2015
//
// Copyright: See COPYING file that comes with this distribution
//

#pragma once

#include <wfc/logger/ilogger.hpp>
#include "writer_options.hpp"
#include <mutex>
#include <atomic>

namespace wfc{ namespace core{

class logger;

class writer
  : public ilogger
{
  typedef std::mutex mutex_type;

public:
  writer(std::shared_ptr<logger> logger);
  void initialize( const writer_options& conf );
  writer_options options() const;
  // ilogger
  virtual void write(const std::string& name, const std::string& ident, std::string str) override;
  
private:
  bool is_deny_(const std::string& some) const;
  void write_to_file_(const std::string& name, const std::string& ident,  const std::string& str);
  void write_to_stdout_(const std::string& name, const std::string& ident,  const std::string& str);
  void write_to_syslog_( const std::string& ident,  const std::string& str);
  
private:
  std::string _filename;
  writer_options _conf;
  std::shared_ptr<logger> _logger;
};

}}
