#pragma once
#include <wfc/statistics/stat_options.hpp>

namespace wfc{ namespace core{

struct statistics_config: 
  ::wfc::statistics::stat_options
{
  // куда отправить агррегированые данные 
  std::string target;
  // куда отправить входящие данные без массива data
  std::vector< std::string > targets;
  time_t aggregate_timeout_ms = 1000;
  time_t startup_ignore_ms = 1000;
};

}}
