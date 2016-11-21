#pragma once

#include <wrtstat/wrtstat_options.hpp>

namespace wfc{

struct stat_config: ::wrtstat::wrtstat_options
{
  std::string log;
  int log_metric = 1;
};

}
