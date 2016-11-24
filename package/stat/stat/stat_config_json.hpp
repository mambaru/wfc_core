#pragma once

#include "stat_config.hpp"
#include <wfc/json.hpp>
#include <wfc/statistics/stat_options_json.hpp>

namespace wfc{

struct stat_config_json
{
  JSON_NAME(perseconds)
  JSON_NAME(seconds)
  JSON_NAME(milliseconds)
  JSON_NAME(microseconds)
  JSON_NAME(nanoseconds)
  JSON_NAME(hide)

  typedef json::enumerator<
    int,
    json::member_list<
      json::enum_value<n_hide,         int, -1>,
      json::enum_value<n_perseconds,   int, 0>,
      json::enum_value<n_nanoseconds,  int, 1>,
      json::enum_value<n_microseconds, int, 1000>,
      json::enum_value<n_milliseconds, int, 1000000>,
      json::enum_value<n_seconds,      int, 1000000000>
    >
  > enum_json;

  JSON_NAME(log)
  JSON_NAME(log_metric)
  JSON_NAME(btp_target)
  
  typedef json::object<
    stat_config,
    json::member_list<
      json::base<stat_options_json>,
      json::member< n_btp_target, stat_config, std::string, &stat_config::btp_target>,
      json::member< n_log,        stat_config, std::string, &stat_config::log>,
      json::member< n_log_metric, stat_config, int,         &stat_config::log_metric, enum_json>
     >
  > type;
  
  typedef type::target target;
  typedef type::serializer serializer;
  typedef type::member_list member_list;
};

}
