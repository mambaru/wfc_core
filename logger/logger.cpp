#include "logger.hpp"

#include <comet/inet/epoller.hpp>
#include <comet/core/global.hpp>
#include <comet/core/imodule.hpp>
#include <comet/core/ilogger.hpp>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <syslog.h>


namespace mamba{ namespace comet{

class log_writer
{
public:
  log_writer(const log_writer_config& config)
    : _config(config)
  {
  }
  
  void initialize(const std::string& name, std::stringstream& str)
  {
    
  }

  void write(const std::string& name, const std::string& ident,  const std::string& str)
  {
    std::cout << "void log_writer::write(const std::string& name, const std::string& ident,  const std::string& str)" << std::endl;
    
    if (!_config.enabled)
      return;

    if ( !_config.path.empty() )
      this->_write_to_file(name, ident, str);
    if ( _config.clog )
      this->_write_to_clog(name, ident, str);
    if ( _config.sylog )
      this->_write_to_sylog(name, ident, str);
  }
  
private:

  void _write_to_file(const std::string& name, const std::string& ident,  const std::string& str)
  {
    std::ofstream oflog( _config.path.c_str(), std::ios_base::app );
    this->_write_to_stream(oflog, name, ident, str);
  }

  void _write_to_clog(const std::string& name, const std::string& ident,  const std::string& str)
  {
    this->_write_to_stream(std::clog, name, ident, str);
  }

  void _write_to_sylog(const std::string& name, const std::string& ident,  const std::string& str)
  {
  }
  
  void _write_to_stream(std::ostream& os, std::string name, std::string ident,  const std::string& str)
  {
    // begin_proccess
    this->_prepare(name, 6);
    this->_prepare(ident, 14);
    
    os << this->_mkdate() << " " << name      << " " << ident     << " " << str;
  }

  void _prepare( std::string& str, size_t width)
  {
    std::transform(str.begin(), str.end(), str.begin(), ::toupper);
    if ( width > str.size() )
      std::fill_n( std::back_inserter(str), width - str.size(), ' ' );
    // fill
    
  }

  std::string _mkdate()
  {
    time_t ts = time(0);
    struct tm t1;
    localtime_r(&ts, &t1);
    char buf[100];
    int sz = strftime(buf,sizeof(buf), "%Y-%m-%d %H:%M:%S",&t1);
    return std::string(buf,sz);
  }

  
private:
  
  log_writer_config _config;
};


logger::~logger()
{
  
}

logger::logger(std::shared_ptr<global> gl)
  : _global(gl)
{
  
}


void logger::initialize(const std::string& name, std::stringstream& str)
{
  std::lock_guard<std::mutex> lk(_mutex);
  
  if ( auto log = this->_find(name))
    log->initialize(name, str);
}

void logger::write(const std::string& name, const std::string& ident,  const std::string& str)
{
  std::cout << "void logger::write(const std::string& name, const std::string& ident,  const std::string& str)" << std::endl;
  std::lock_guard<std::mutex> lk(_mutex);
  
  if ( auto log = this->_find(name))
    log->write(name, ident, str);
  else
    std::cout << "not found " << name << " " << _log_map.size() << std::endl;
  
}

void logger::configure(const logger_config& conf)
{
  std::lock_guard<std::mutex> lk(_mutex);
  
  _config = conf;
  _log_map.clear();
  
  _create_log("config", _config.config);
  _create_log("daemon", _config.daemon);
  _create_log("common", _config.common);
  _create_log("debug",  _config.debug);
  _create_log("trace",  _config.trace);
}

/// 
/// private
///

std::shared_ptr<log_writer> logger::_find(const std::string& name) const
{
  std::cout << "find " << name << " " << _log_map.size() << std::endl;
  auto itr = _log_map.find(name);
  if ( itr==_log_map.end() )
    return nullptr;
  return itr->second;
}

void logger::_create_log(const std::string& name, const log_writer_config& lwc)
{
  _log_map[name] = std::make_shared<log_writer>(lwc);
  std::cout << "registry " << name << " " << _log_map.size() << std::endl;
}

/*void logger::_create_log(const std::string& name, std::string filename, bool clog, bool sylog)
{
  if ( filename.empty() )
    return;
  
  if ( filename != "-" )
    _tmp_filename = filename;
  else
    filename = _tmp_filename;
  
  if ( filename == "clog" )
  {
    clog = true;
    filename.clear();
  }

  if ( filename == "syslog" )
  {
    sylog = true;
    filename.clear();
  }

  _log_map[name] = std::make_shared<log_writer>(filename, clog, sylog);
  
}
*/

}}
