#pragma once

namespace mamba{ namespace comet{

struct logger_config
{
  bool sylog;
  time_t lifetime;
  std::string path;
  std::string stdout;
  logger_config()
    : sylog(false)
    , lifetime(0)
    , stdout("clog")
  {}
};

}}
