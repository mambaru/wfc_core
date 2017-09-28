
#pragma once

namespace wfc{ namespace core{

struct statistics_config
{
  std::string target;
  time_t aggregate_timeout_ms = 1000;
  time_t startup_ignore_ms = 1000;
};

}}
