#pragma once

namespace wfc{

struct core_config
{
  bool enabled = false;
  time_t idle_timeout_ms = 1000;
  time_t wait_timeout_ms = 200;
  std::shared_ptr<size_t> rlimit_as_gb;
};

}
