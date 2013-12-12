#pragma once

namespace wfc{

struct core_config
{
  bool enabled;
  time_t idle_timeout_ms;
  time_t wait_timeout_ms;
  core_config()
    : enabled(true)
    , idle_timeout_ms(1000)
    , wait_timeout_ms(200)
  {}
};

}
