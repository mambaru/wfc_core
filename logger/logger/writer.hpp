//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2013-2015
//
// Copyright: See COPYING file that comes with this distribution
//

#pragma once

#include <wfc/logger/ilogger.hpp>
#include "writer_options.hpp"
#include <mutex>

namespace wfc{

class writer
  : public ilogger
{
public:
  writer();

  void initialize( const writer_options& conf );
  
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
  writer_options _conf;
  size_t _summary;
  std::string _starttime;
 
};

}
