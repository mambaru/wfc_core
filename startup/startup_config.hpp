//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2013-2015
//
// Copyright: See COPYING file that comes with this distribution
//
#pragma once

#include <ctime>

namespace wfc{

struct startup_config
{
  bool enabled;
  time_t idle_timeout_ms;
  time_t wait_timeout_ms;
  startup_config()
    : enabled(true)
    , idle_timeout_ms(1000)
    , wait_timeout_ms(200)
  {}
};

}
