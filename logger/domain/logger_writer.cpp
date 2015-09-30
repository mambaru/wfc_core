//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2013-2015
//
// Copyright: See COPYING file that comes with this distribution
//

#include "logger_writer.hpp"

#include <string>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <ctime>
#include <cctype>
#include <syslog.h>
#include <boost/concept_check.hpp>


namespace wfc{

namespace
{
  void prepare( std::string& str, size_t width)
  {
    std::transform(str.begin(), str.end(), str.begin(), [](char ch)->char{ return std::toupper(ch);} );
    if ( width > str.size() )
      std::fill_n( std::back_inserter(str), width - str.size(), ' ' );
  }

  std::string mkdate()
  {
    time_t ts = time(0);
    struct tm t1;
    localtime_r(&ts, &t1);
    char buf[100];
    int sz = strftime(buf,sizeof(buf), "%Y-%m-%d %H:%M:%S",&t1);
    return std::string(buf,sz);
  }
  
  void write_to_stream(std::ostream& os, std::string name, std::string ident,  const std::string& str)
  {
    prepare(name, 6);
    prepare(ident, 9);
    os << mkdate() << " " << name << " " << ident << " " << str;
  }

  
  size_t name2pri(const std::string& name)
  {
    if (name=="ERROR")    return LOG_ERR;
    if (name=="WARNING")  return LOG_WARNING;
    if (name=="MESSAGE")  return LOG_NOTICE;
    if (name=="FATAL")    return LOG_ALERT;
    if (name=="BEGIN")    return LOG_INFO;
    if (name=="END")      return LOG_INFO;
    if (name=="DEBUG")    return LOG_DEBUG;
    if (name=="TRACE")    return LOG_DEBUG;
    if (name=="PROGRESS") return LOG_INFO;
    return LOG_ERR;
  }
  
  inline bool replace(std::string& str, const std::string& from, const std::string& to) 
  {
    size_t start_pos = str.find(from);
    if(start_pos == std::string::npos)
        return false;
    str.replace(start_pos, from.length(), to);
    return true;
  }

} // namespace


logger_writer::logger_writer()
  : _summary(0)
  , _starttime(mkdate())
{
}

void logger_writer::initialize(const writer_config& conf)
{
  std::lock_guard<mutex_type> lk(_mutex);
  _conf = conf;
  std::sort( _conf.deny.begin(), _conf.deny.end() );
}

bool logger_writer::is_deny_(const std::string& some) const
{
  return std::find( _conf.deny.begin(), _conf.deny.end(), some ) != _conf.deny.end();
}

void logger_writer::write(const std::string& name, const std::string& ident,  std::string str)
{
  if (is_deny_(name) || is_deny_(ident))
    return;
  
  while ( replace(str, "\r\n", "\\r\\n") );

  std::lock_guard<mutex_type> lk(_mutex);
  
  if ( !_conf.path.empty() )
  {
    this->write_to_file_(name, ident, str);
  }
  
  if ( !_conf.stdout.empty() )
  {
    this->write_to_stdout_(name, ident, str);
  }
  
  if ( !_conf.syslog.empty() && name == "syslog")
  {
    this->write_to_syslog_(ident, str);
  }
}

void logger_writer::write_to_file_(const std::string& name, const std::string& ident,  const std::string& str)
{
  std::ofstream oflog( _conf.path, std::ios_base::app );
   
  if ( _conf.limit > 0 )
  {
    size_t size = oflog.tellp();
    if ( size > _conf.limit )
    {
      _summary += size;
      oflog.close();
      oflog.open(_conf.path);
      oflog << "---------------- truncate with " << size 
            << " summary size " << _summary 
            << " ( start time: " << _starttime << ")"
            << " ----------------" << std::endl;
    }
  }

  write_to_stream(oflog, name, ident, str);
}

void logger_writer::write_to_stdout_(const std::string& name, const std::string& ident,  const std::string& str)
{
  std::ostream *p = nullptr;
  if (_conf.stdout=="cout")
    p = &std::cout;
  else if (_conf.stdout=="clog")
    p = &std::clog;
  else if (_conf.stdout=="cerr")
    p = &std::cerr;
  
  if (p)
    write_to_stream(*p, name, ident, str);
}

void logger_writer::write_to_syslog_(const std::string& ident, const std::string& str)
{
  ::openlog( _conf.syslog.c_str(), 0, LOG_USER);
  ::syslog(name2pri(ident), str.c_str());
  ::closelog();
}

}
