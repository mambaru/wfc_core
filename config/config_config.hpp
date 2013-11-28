#pragma once

namespace mamba{ namespace comet{

struct config_config
{
  bool enabled;
  bool reload_changed;
  config_config()
    : enabled(true)
    , reload_changed(false)
  {}
};

}}
