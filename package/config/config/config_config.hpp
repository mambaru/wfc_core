#pragma once

namespace wfc{

struct config_config
{
  bool reload_sighup = false;
  time_t reload_changed_ms = 0;
  };

}
