#pragma once
#include <string>

namespace mamba{ namespace comet{

/*
struct log_writer_config
{
  bool enabled;
  bool sylog;
  bool clog;
  std::string path;
  
  log_writer_config(const std::string& path = std::string() )
    : enabled(true)
    , sylog(false)
    , clog(true)
    , path(path)
  {}
};*/

struct logger_module_config
{
  
  bool enabled;
  bool sylog;
  bool single;
  time_t lifetime;
  std::string stdout;
  std::string prefix;
  
  logger_module_config()
    : enabled(true)
    , sylog(false)
    , single(true)
    , lifetime(0)
    , stdout("clog")
    , prefix("")
  {}
};

}}
