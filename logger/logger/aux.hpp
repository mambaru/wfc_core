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


namespace wfc{ namespace aux{

  //void prepare( std::string& str, size_t width);

  std::string mkdate();
  
  // std::string mkms(bool enable);
  
  void write_to_stream(std::ostream& os, std::string name, std::string ident,  const std::string& str, bool milliseconds );
  
  // size_t type2pri(const std::string& name);
  
  bool replace(std::string& str, const std::string& from, const std::string& to);

  void syslog_write(const std::string& sysname, const std::string& type, const std::string& str);
  
}} // namespace
