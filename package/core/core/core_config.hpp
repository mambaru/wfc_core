#pragma once

#include <ctime>
#include <wfc/core/workflow_options.hpp>

namespace wfc{

struct core_config
{
  time_t core_timeout_ms = 1000;
  time_t idle_timeout_ms = 1000;
  size_t rlimit_as_mb = 0;
  bool enable_callback_check = true;
  workflow_options workflow;
};

}
