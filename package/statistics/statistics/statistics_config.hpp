//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2013-2018
//
// Copyright: See COPYING file that comes with this distribution
//

#pragma once

#include <wfc/statistics/statistics_options.hpp>

namespace wfc{ namespace core{

struct statistics_config:
  ::wfc::statistics::statistics_options
{
  // куда отправить входящие данные без массива data
  std::vector< std::string > targets;
  time_t aggregate_timeout_ms = 1000;
  time_t startup_ignore_ms = 1000;
  bool suspend_push = false;
};

struct statistics_statistics_config
{
  std::string multi_push_meter = "multi_push.time";
  std::string multi_count_meter = "multi_push.values";
  std::string push_meter = "push.time";
  std::string count_meter = "push.values";
};

}}
