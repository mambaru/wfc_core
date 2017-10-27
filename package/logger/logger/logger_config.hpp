//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2013-2015
//
// Copyright: See COPYING file that comes with this distribution
//

#pragma once

#include "writer_options.hpp"

#include <string>
#include <unordered_map>

namespace wfc{ namespace core{

struct logger_config
  : writer_options
{
  typedef std::unordered_map<std::string, writer_options> custom_map;
  bool single = true;
  bool abort_with_fatal_message = false;
  custom_map custom;
};

}}
