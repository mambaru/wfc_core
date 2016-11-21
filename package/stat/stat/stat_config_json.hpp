#pragma once

#include "stat_config.hpp"
#include <wfc/json.hpp>

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

  JSON_NAME(reduced_size)
  JSON_NAME(step_ns)
  JSON_NAME(limit)
  JSON_NAME(levels)
  JSON_NAME(log)
  JSON_NAME(log_metric)
  
  typedef json::object<
    stat_config,
    json::member_list<
      json::member< n_reduced_size,  ::wrtstat::aggregator_options, ::wrtstat::types::size_type, & ::wrtstat::aggregator_options::reduced_size>,
      json::member< n_step_ns, ::wrtstat::separator_options, ::wrtstat::types::time_type, &::wrtstat::separator_options::step_ts>,
      json::member< n_limit, ::wrtstat::reducer_options, ::wrtstat::types::size_type, &::wrtstat::reducer_options::limit>,
      json::member< n_levels, ::wrtstat::reducer_options, ::wrtstat::types::size_type, &::wrtstat::reducer_options::levels>,
      json::member< n_log, stat_config, std::string, &stat_config::log>,
      json::member< n_log_metric, stat_config, int, &stat_config::log_metric, enum_json>
     >
  > type;
  
  typedef type::target target;
  typedef type::serializer serializer;
  typedef type::member_list member_list;
};

}
