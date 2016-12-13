#pragma once

namespace wfc{ namespace core{

struct system_statistics_config
{
  std::string prefix;
  time_t interval_ms = 1000;
};

}}
