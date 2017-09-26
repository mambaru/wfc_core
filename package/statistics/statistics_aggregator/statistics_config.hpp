
#pragma once

namespace wfc{ namespace core{

struct statistics_config
{
  std::string btp_target;
  std::string log;
  int log_metric = 1;
  time_t timeout_ms = 1000;
  time_t btp_delay_ms = 30000;
};

}}
