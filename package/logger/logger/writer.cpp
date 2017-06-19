//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2013-2015
//
// Copyright: See COPYING file that comes with this distribution
//

#include "writer.hpp"
#include "logger.hpp"
#include "aux.hpp"

#include <string>
#include <memory>
#include <iostream>
#include <fstream>

namespace wfc{ namespace core{

static std::mutex stdout_mutex;

writer_options writer::options() const
{
  return _conf;
}

writer::writer(std::shared_ptr<logger> logger)
  : _logger(logger)
{
}

void writer::initialize(const writer_options& conf)
{
  _conf = conf;
  std::sort( _conf.deny.begin(), _conf.deny.end() );
}

void writer::write(const std::string& name, const std::string& ident,  std::string str)
{
  if (is_deny_(name) || is_deny_(ident))
    return;
  
  while ( aux::replace(str, "\r\n", "\\r\\n") );

  if ( !_conf.path.empty() )
  {
    std::lock_guard<mutex_type> lk( _logger->_mutex);
    this->write_to_file_(name, ident, str);
  }
  
  if ( !_conf.stdout.empty() )
  {
    std::lock_guard< std::mutex> lk(stdout_mutex);
    this->write_to_stdout_(name, ident, str);
  }
  
  if ( !_conf.syslog.empty() && name == "syslog")
  {
    this->write_to_syslog_(ident, str);
  }
}

bool writer::is_deny_(const std::string& some) const
{
  return std::find( _conf.deny.begin(), _conf.deny.end(), some ) != _conf.deny.end();
}

void writer::write_to_file_(const std::string& name, const std::string& ident,  const std::string& str)
{
  std::ofstream oflog( _conf.path, std::ios_base::app );
   
  if ( _conf.limit > 0 )
  {
    size_t size = oflog.tellp();
    if ( size > _conf.limit )
    {
      _logger->_summary += size;
      oflog.close();
      oflog.open(_conf.path);
      oflog << "---------------- truncate with " << size 
            << " summary size " << _logger->_summary 
            << " ( start time: " << _logger->_starttime << ")"
            << " ----------------" << std::endl;
    }
  }

  aux::write_to_stream(oflog, name, ident, str, _conf.milliseconds);
  oflog.close();
}

void writer::write_to_stdout_(const std::string& name, const std::string& ident,  const std::string& str)
{
  std::ostream *p = nullptr;
  if (_conf.stdout=="cout")
    p = &std::cout;
  else if (_conf.stdout=="clog")
    p = &std::clog;
  else if (_conf.stdout=="cerr")
    p = &std::cerr;
  
  if (p)
  {
    aux::write_to_stream(*p, name, ident, str, _conf.milliseconds);
    p->flush();
  }
}

void writer::write_to_syslog_(const std::string& ident, const std::string& str)
{
  aux::syslog_write(_conf.syslog, ident, str);
}

}}
