//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2013-2015
//
// Copyright: See COPYING file that comes with this distribution
//

#include "writer.hpp"

#include <chrono>
#include <iomanip>
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


namespace wfc{ namespace aux {
  
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
  
  std::string mkms(bool enable)
  {
    if (!enable) return std::string();
    
    auto duration = std::chrono::high_resolution_clock::now().time_since_epoch();
    auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
    auto secround = (millis/1000) * 1000;
    std::stringstream ss;
    ss << "." << std::setfill('0') << std::setw(3)<< millis - secround;
    return ss.str();
  }
  
  void write_to_stream(std::ostream& os, std::string name, std::string ident,  const std::string& str, bool milliseconds )
  {
    prepare(name, 6);
    prepare(ident, 9);
    os << mkdate() << mkms(milliseconds) << " " << name << " " << ident << " " << str;
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
  
  bool replace(std::string& str, const std::string& from, const std::string& to) 
  {
    size_t start_pos = str.find(from);
    if(start_pos == std::string::npos)
        return false;
    str.replace(start_pos, from.length(), to);
    return true;
  }

  
} }
