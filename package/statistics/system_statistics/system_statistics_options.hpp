#pragma once

namespace wfc{ namespace core{

struct system_statistics_options
{
  std::string prefix;
  time_t interval_ms = 1000;
};

}}
