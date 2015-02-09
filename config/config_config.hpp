#pragma once

namespace wfc{

struct config_config
{
  //bool enabled;
  bool reload_changed;
  bool reload_sighup;
  config_config()
    : /*enabled(true)
    ,*/
      reload_changed(false)
    , reload_sighup(false)
  {}
};

}
