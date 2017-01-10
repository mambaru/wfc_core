#pragma once

#include "statistics_config.hpp"
#include <wfc/json.hpp>
#include <wfc/statistics/stat_options_json.hpp>

namespace wfc{ namespace core{

struct statistics_config_json
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
  JSON_NAME(timeout_ms)
  JSON_NAME(btp_delay_ms)
  
  typedef json::object<
    statistics_config,
    json::member_list<
      //json::base<stat_options_json>,
      json::member< n_timeout_ms,   statistics_config, time_t,      &statistics_config::timeout_ms>,
      json::member< n_btp_delay_ms, statistics_config, time_t,      &statistics_config::btp_delay_ms>,
      json::member< n_btp_target,   statistics_config, std::string, &statistics_config::btp_target>,
      json::member< n_log,          statistics_config, std::string, &statistics_config::log>,
      json::member< n_log_metric,   statistics_config, int,         &statistics_config::log_metric, enum_json>
     >
  > type;
  
  typedef type::target target;
  typedef type::serializer serializer;
  typedef type::member_list member_list;
};

}}
