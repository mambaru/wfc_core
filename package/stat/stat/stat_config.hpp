#pragma once

#include <wrtstat/wrtstat_options.hpp>

namespace wfc{

struct stat_config
  : public ::wrtstat::wrtstat_options
{
  std::string btp_target;
  std::string log;
  int log_metric = 1;
};

}
