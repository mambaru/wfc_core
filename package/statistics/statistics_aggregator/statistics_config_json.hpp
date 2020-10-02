//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2013-2018
//
// Copyright: See COPYING file that comes with this distribution
//

#pragma once

#include "statistics_config.hpp"
#include <wfc/json.hpp>
#include <wfc/statistics/stat_options_json.hpp>

namespace wfc{ namespace core{

struct aggreagtor_config_json
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
    aggreagtor_config,
    json::member_list<
      json::base<wrtstat::wrtstat_options_json>,
      /*json::member< n_target,   aggreagtor_config, std::string, &aggreagtor_config::target>,*/
      json::member< n_targets,  aggreagtor_config, std::vector<std::string>, &aggreagtor_config::targets, 
                    json::vector_of_strings<> >,
      json::member< n_workers,   aggreagtor_config, size_t, &aggreagtor_config::workers>,
      json::member< n_suspend_push,   aggreagtor_config, bool, &aggreagtor_config::suspend_push>,
      //json::member< n_hash_size,   aggreagtor_config, size_t, &aggreagtor_config::hash_size>,
      json::member< n_aggregate_timeout_ms, aggreagtor_config, time_t, &aggreagtor_config::aggregate_timeout_ms>,
      json::member< n_startup_ignore_ms, aggreagtor_config, time_t,      &aggreagtor_config::startup_ignore_ms>
      
     >,
    json::strict_mode
  > type;
  
  typedef type::target target;
  typedef type::serializer serializer;
  typedef type::member_list member_list;
};


struct aggreagtor_statistics_config_json
{
  JSON_NAME2(n_multi_push_meter, "multi_push.time")
  JSON_NAME2(n_multi_count_meter, "multi_push.values")
  JSON_NAME2(n_push_meter, "push.time")
  JSON_NAME2(n_count_meter, "push.values")
  
  typedef json::object<
    aggreagtor_statistics_config,
    json::member_list<
      json::member< n_multi_push_meter,   aggreagtor_statistics_config, std::string, &aggreagtor_statistics_config::multi_push_meter>,
      json::member< n_multi_count_meter,   aggreagtor_statistics_config, std::string, &aggreagtor_statistics_config::multi_count_meter>,
      json::member< n_push_meter,   aggreagtor_statistics_config, std::string, &aggreagtor_statistics_config::push_meter>,
      json::member< n_count_meter,   aggreagtor_statistics_config, std::string, &aggreagtor_statistics_config::count_meter>
    >,
    json::strict_mode
  > type;
  
  typedef type::target target;
  typedef type::serializer serializer;
  typedef type::member_list member_list;
};

}}
