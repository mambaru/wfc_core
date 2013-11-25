#pragma once
#include <string>

namespace mamba{ namespace comet{

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
};

struct logger_config
{
  bool enabled;
  
  log_writer_config config;
  log_writer_config daemon;
  log_writer_config common;
  log_writer_config debug;
  log_writer_config trace;
  
  logger_config()
    : enabled(true)
    , config("./default.config.log")
    , daemon("./default.daemon.log")
    , common("./default.common.log")
    , debug("./default.debug.log")
    , trace("./default.trace.log")
  {}
};

}}
