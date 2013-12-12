#pragma once

namespace wfc{

struct config_config
{
  bool enabled;
  bool reload_changed;
  config_config()
    : enabled(true)
    , reload_changed(false)
  {}
};

}
