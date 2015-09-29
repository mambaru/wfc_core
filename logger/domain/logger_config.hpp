//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2013-2015
//
// Copyright: See COPYING file that comes with this distribution
//

#pragma once

#include "writer_config.hpp"
#include <string>
#include <vector>

namespace wfc{

struct logger_config
  : writer_config
{
  bool single = true;
  std::vector<std::string> reject;
};

}
