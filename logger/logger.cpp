#include "logger.hpp"

//#include <wfc/inet/epoller.hpp>
#include <wfc/core/global.hpp>
#include <wfc/module/imodule.hpp>
#include <wfc/logger/ilogger.hpp>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <syslog.h>


namespace wfc{
  
namespace {
  void prepare( std::string& str, size_t width)
  {
    std::transform(str.begin(), str.end(), str.begin(), ::toupper);
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
    // begin_proccess
    prepare(name, 6);
    prepare(ident, 9);
    os << mkdate() << " " << name << " " << ident << " " << str;
  }

  
  void write_to_file(const std::string& path, const std::string& name, const std::string& ident,  const std::string& str)
  {
    std::ofstream oflog( path, std::ios_base::app );
    write_to_stream(oflog, name, ident, str);
  }

  void write_to_stdout(const std::string& stdout, const std::string& name, const std::string& ident,  const std::string& str)
  {
    std::ostream *p = nullptr;
    if (stdout=="cout")
      p = &std::cout;
    else if (stdout=="clog")
      p = &std::clog;
    else if (stdout=="cerr")
      p = &std::cerr;
    
    if (p)
      write_to_stream(*p, name, ident, str);
  }

  void write_to_sylog(const std::string& /*name*/, const std::string& /*ident*/,  const std::string& /*str*/)
  {
    // TODO:
  }
  
} // namespace

logger::logger(const logger_config& conf)
  : _conf(conf)
{
}

void logger::initialize(const std::string& /*name*/, std::stringstream& /*str*/)
{
  std::lock_guard<std::mutex> lk(_mutex);
}

void logger::write(const std::string& name, const std::string& ident,  const std::string& str)
{
  std::lock_guard<std::mutex> lk(_mutex);
  if ( !_conf.path.empty() )
    write_to_file(_conf.path, name, ident, str);
  if ( !_conf.stdout.empty() )
    write_to_stdout(_conf.stdout, name, ident, str);
  if ( _conf.sylog )
    write_to_sylog(name, ident, str);
}

}
