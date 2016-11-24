#pragma once

#include <wfc/statistics/stat_options.hpp>

namespace wfc{

struct stat_config
  : public stat_options
{
  std::string btp_target;
  std::string log;
  int log_metric = 1;
};

}
