#pragma once

namespace wfc{

struct config_config
{
  bool reload_changed;
  bool reload_sighup;
  config_config()
    : reload_changed(false)
    , reload_sighup(false)
  {}
};

}
