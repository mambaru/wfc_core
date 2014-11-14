#pragma once

#include "logger_module_config.hpp"
#include <wfc/json/json.hpp>
#include <wfc/json/name.hpp>

namespace wfc{

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
  JSON_NAME(enabled)
  JSON_NAME(single)
  JSON_NAME(syslog)
  JSON_NAME(lifetime)
  JSON_NAME(stdout)
  JSON_NAME(prefix)

  typedef json::object<
    logger_module_config,
    fas::type_list_n<
      json::member<n_enabled, logger_module_config, bool,        &logger_module_config::enabled>,
      json::member<n_single,  logger_module_config, bool,        &logger_module_config::single>,
      json::member<n_syslog,  logger_module_config, bool,        &logger_module_config::sylog>,
      json::member<n_lifetime,logger_module_config, time_t,      &logger_module_config::lifetime>,
      json::member<n_stdout,  logger_module_config, std::string, &logger_module_config::stdout>,
      json::member<n_prefix,  logger_module_config, std::string, &logger_module_config::prefix>
    >::type
  > type;
  
  typedef type::serializer serializer;
  typedef type::target target;
  
};

}
