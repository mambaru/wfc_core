#pragma once

#include "logger_module_config.hpp"
#include <comet/json/json.hpp>
#include <comet/json/name.hpp>

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
};
*/

  
struct logger_module_config_json
{
  NAME(enabled)
  NAME(syslog)
  NAME(clog)
  NAME(path)

  NAME(config)
  NAME(daemon)
  NAME(common)
  NAME(debug)
  NAME(trace)

  /*
  typedef json::object<
    log_writer_config,
    type_list_n<
       json::member<n_enabled, log_writer_config, bool, &log_writer_config::enabled>,
       json::member<n_path, log_writer_config, std::string, &log_writer_config::path>,
       json::member<n_syslog, log_writer_config, bool, &log_writer_config::sylog>,
       json::member<n_clog, log_writer_config, bool, &log_writer_config::clog>
    >::type
  > log_writer_config_json;*/
  
  typedef json::object<
    logger_module_config,
    type_list_n<
        /*
       json::member<n_enabled, logger_config, bool, &logger_config::enabled>,
       json::member<n_config,  logger_config, log_writer_config, &logger_config::config, log_writer_config_json>,
       json::member<n_daemon,  logger_config, log_writer_config, &logger_config::daemon, log_writer_config_json>,
       json::member<n_common,  logger_config, log_writer_config, &logger_config::common, log_writer_config_json>,
       json::member<n_debug,   logger_config, log_writer_config, &logger_config::debug,  log_writer_config_json>,
       json::member<n_trace,   logger_config, log_writer_config, &logger_config::trace,  log_writer_config_json>
       */
    >::type
  >::serializer serializer;
  
  
};

}}
