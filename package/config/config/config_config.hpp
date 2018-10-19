//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2013-2018
//
// Copyright: See COPYING file that comes with this distribution
//

#pragma once

namespace wfc{ namespace core{

struct config_config
{
  bool reload_sighup = false;
  time_t reload_changed_ms = 0;
};

}}
