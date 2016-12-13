#pragma once

#include <wfc/statistics/stat_options.hpp>

namespace wfc{ namespace core{

struct statistics_config
  : public stat_options
{
  std::string btp_target;
  std::string log;
  int log_metric = 1;
};

}}
