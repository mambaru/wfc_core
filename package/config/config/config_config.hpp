//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2013-2018, 2022
//
// Copyright: See COPYING file that comes with this distribution
//

#pragma once
#include <vector>
#include <string>

namespace wfc{ namespace core{

struct config_config
{
  bool show_config = false;
  bool reload_sighup = false;
  time_t reload_changed_ms = 0;
  std::vector<std::string> ini;
};

}}
