//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2013-2018
//
// Copyright: See COPYING file that comes with this distribution
//

#pragma once

#include <string>
#include <vector>
#include <utility>
#include <algorithm>

namespace wfc{ namespace core{

typedef std::pair<std::string, std::string> conf_pair;
typedef std::vector< conf_pair > configuration;


inline configuration::const_iterator conf_find( const std::string& name, const configuration& conf)
{
  return std::find_if(conf.begin(), conf.end(), [&name](const configuration::value_type& cp){
    return cp.first == name;
  });
}

inline configuration::iterator conf_find( const std::string& name, configuration& conf)
{
  return std::find_if(conf.begin(), conf.end(), [&name](const configuration::value_type& cp){
    return cp.first == name;
  });
}

inline size_t conf_count( const std::string& name, const configuration& conf )
{
  return static_cast<size_t>(
    std::count_if(conf.begin(), conf.end(), [&name](const configuration::value_type& cp){
      return cp.first == name;
    })
  );
}



}}
