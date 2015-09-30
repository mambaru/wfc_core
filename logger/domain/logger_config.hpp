//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2013-2015
//
// Copyright: See COPYING file that comes with this distribution
//

#pragma once

#include "writer_config.hpp"
#include <string>
#include <vector>
#include <unordered_map>
namespace wfc{

struct logger_config
  : writer_config
{
  typedef std::unordered_map<std::string, writer_config> custom_map;
  bool single = true;
  custom_map custom;
};

}
