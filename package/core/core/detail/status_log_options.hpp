#pragma once

#include <ctime>

namespace wfc{  namespace core{

struct status_log_options
{
  time_t hang_timeout_ms = 60000;
  time_t errors_ttl_ms = 60000;
  time_t warnings_ttl_ms = 10000;
  size_t errors_limit = 10;
  size_t warnings_limit = 100;
};

}}
