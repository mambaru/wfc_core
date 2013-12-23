#pragma once

namespace wfc{

struct startup_config
{
  bool enabled;
  time_t idle_timeout_ms;
  time_t wait_timeout_ms;
  startup_config()
    : enabled(true)
    , idle_timeout_ms(1000)
    , wait_timeout_ms(200)
  {}
};

}
