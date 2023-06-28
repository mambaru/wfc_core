//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2013-2018
//
// Copyright: See COPYING file that comes with this distribution
//

#pragma once

#include "statistics_config.hpp"
#include <wfc/json.hpp>
#include <wfc/statistics/statistics_options_json.hpp>

namespace wfc{ namespace core{

struct statistics_config_json
{
  JSON_NAME(targets)
  JSON_NAME(aggregate_timeout_ms)
  JSON_NAME(startup_ignore_ms)
  JSON_NAME(suspend_push)

  typedef json::object<
    statistics_config,
    json::member_list<
      json::base< ::wfc::statistics::statistics_options_json>,
      json::member< n_targets,  statistics_config, std::vector<std::string>, &statistics_config::targets,
                    json::vector_of_strings<> >,
      json::member< n_suspend_push,   statistics_config, bool, &statistics_config::suspend_push>,
      json::member< n_aggregate_timeout_ms, statistics_config, time_t, &statistics_config::aggregate_timeout_ms, wjson::time_interval_ms<> >,
      json::member< n_startup_ignore_ms, statistics_config, time_t,      &statistics_config::startup_ignore_ms, wjson::time_interval_ms<> >
      
     >,
    json::strict_mode
  > meta;
  
  typedef meta::target target;
  typedef meta::serializer serializer;
  typedef meta::member_list member_list;
};


struct statistics_statistics_config_json
{
  JSON_NAME2(n_multi_push_meter, "multi_push.time")
  JSON_NAME2(n_multi_count_meter, "multi_push.values")
  JSON_NAME2(n_push_meter, "push.time")
  JSON_NAME2(n_count_meter, "push.values")
  
  typedef json::object<
    statistics_statistics_config,
    json::member_list<
      json::member< n_multi_push_meter,   statistics_statistics_config, std::string, &statistics_statistics_config::multi_push_meter>,
      json::member< n_multi_count_meter,   statistics_statistics_config, std::string, &statistics_statistics_config::multi_count_meter>,
      json::member< n_push_meter,   statistics_statistics_config, std::string, &statistics_statistics_config::push_meter>,
      json::member< n_count_meter,   statistics_statistics_config, std::string, &statistics_statistics_config::count_meter>
    >,
    json::strict_mode
  > meta;
  
  typedef meta::target target;
  typedef meta::serializer serializer;
  typedef meta::member_list member_list;
};

}}
