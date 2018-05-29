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

  JSON_NAME(target)
  JSON_NAME(targets)
  JSON_NAME(workers)
  JSON_NAME(hash_size)
  JSON_NAME(aggregate_timeout_ms)
  JSON_NAME(startup_ignore_ms)
  JSON_NAME(suspend_push)

  
  typedef json::object<
    statistics_config,
    json::member_list<
      json::base<wrtstat::wrtstat_options_json>,
      /*json::member< n_target,   statistics_config, std::string, &statistics_config::target>,*/
      json::member< n_targets,  statistics_config, std::vector<std::string>, &statistics_config::targets, 
                    json::vector_of_strings<> >,
      json::member< n_workers,   statistics_config, size_t, &statistics_config::workers>,
      json::member< n_suspend_push,   statistics_config, bool, &statistics_config::suspend_push>,
      //json::member< n_hash_size,   statistics_config, size_t, &statistics_config::hash_size>,
      json::member< n_aggregate_timeout_ms, statistics_config, time_t, &statistics_config::aggregate_timeout_ms>,
      json::member< n_startup_ignore_ms, statistics_config, time_t,      &statistics_config::startup_ignore_ms>
      
     >,
    json::strict_mode
  > type;
  
  typedef type::target target;
  typedef type::serializer serializer;
  typedef type::member_list member_list;
};

}}
