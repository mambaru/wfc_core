#pragma once

#include <ctime>

namespace wfc{

struct core_config
{
  time_t idle_timeout_ms = 1000;
  size_t rlimit_as_mb = 0;
};

}
