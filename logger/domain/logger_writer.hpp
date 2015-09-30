//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2013-2015
//
// Copyright: See COPYING file that comes with this distribution
//

#pragma once

#include <wfc/logger/ilogger.hpp>
#include "writer_config.hpp"
#include <mutex>

namespace wfc{

class logger_writer
  : public ilogger
{
public:
  logger_writer();

  void initialize( const writer_config& conf );
  
  virtual void write(const std::string& name, const std::string& ident, std::string str);
  
private:
  bool is_deny_(const std::string& some) const;
  void write_to_file_(const std::string& name, const std::string& ident,  const std::string& str);
  void write_to_stdout_(const std::string& name, const std::string& ident,  const std::string& str);
  void write_to_syslog_( const std::string& ident,  const std::string& str);
private:
  typedef std::recursive_mutex mutex_type;
  mutex_type _mutex;
  std::string _filename;
  writer_config _conf;
  size_t _summary;
  std::string _starttime;
 
};

}
