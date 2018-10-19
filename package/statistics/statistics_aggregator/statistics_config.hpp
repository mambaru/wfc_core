//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2013-2018
//
// Copyright: See COPYING file that comes with this distribution
//

#pragma once

#include <wfc/statistics/stat_options.hpp>

namespace wfc{ namespace core{

struct statistics_config: 
  wrtstat::wrtstat_options
{
  // куда отправить агррегированые данные 
  // std::string target;
  // куда отправить входящие данные без массива data
  std::vector< std::string > targets;
  size_t workers = 0;
  //size_t hash_size = 0;
  time_t aggregate_timeout_ms = 1000;
  time_t startup_ignore_ms = 1000;
  bool suspend_push = false;
};

}}
