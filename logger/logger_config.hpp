#pragma once
#include <string>

namespace wfc{

struct logger_config
{
  bool enabled;
  bool sylog;
  bool single;
  time_t lifetime;
  std::string stdout;
  std::string prefix;
  
  logger_config()
    : enabled(true)
    , sylog(false)
    , single(true)
    , lifetime(0)
    , stdout("clog")
    , prefix("")
  {}
};

}
