#pragma once

namespace mamba{ namespace comet{

struct core_config
{
  bool enabled;
  int idle_timeout_ms;
  int wait_timeout_ms;
  core_config()
    : enabled(true)
    , idle_timeout_ms(1000)
    , wait_timeout_ms(200)
  {}
};

}}
