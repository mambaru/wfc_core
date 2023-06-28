//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2013-2018
//
// Copyright: See COPYING file that comes with this distribution
//

#pragma once

#include "aggregator_config.hpp"
#include <wfc/json.hpp>
#include <wrtstat/multi_aggregator/multi_aggregator_options_json.hpp>

namespace wfc{ namespace core{

struct aggregator_config_json
{
  JSON_NAME(targets)
  JSON_NAME(pushout_timer_ms)
  JSON_NAME(suspend_push)

  
  typedef json::object<
    aggregator_config,
    json::member_list<
      json::base<wrtstat::multi_aggregator_options_json>,
      json::member< n_targets,  aggregator_config, std::vector<std::string>, &aggregator_config::targets,
                    json::vector_of_strings<> >,
      json::member< n_suspend_push,   aggregator_config, bool, &aggregator_config::suspend_push>,
      json::member< n_pushout_timer_ms, aggregator_config, time_t, &aggregator_config::pushout_timer_ms, wjson::time_interval_ms<> >
     >,
    json::strict_mode
  > meta;
  
  typedef meta::target target;
  typedef meta::serializer serializer;
  typedef meta::member_list member_list;
};


struct aggregator_statistics_config_json
{
  JSON_NAME2(n_multi_push_meter, "multi_push.time")
  JSON_NAME2(n_multi_count_meter, "multi_push.values")
  JSON_NAME2(n_push_meter, "push.time")
  JSON_NAME2(n_count_meter, "push.values")
  
  typedef json::object<
    aggregator_statistics_config,
    json::member_list<
      json::member< n_multi_push_meter,   aggregator_statistics_config, std::string, &aggregator_statistics_config::multi_push_meter>,
      json::member< n_multi_count_meter,   aggregator_statistics_config, std::string, &aggregator_statistics_config::multi_count_meter>,
      json::member< n_push_meter,   aggregator_statistics_config, std::string, &aggregator_statistics_config::push_meter>,
      json::member< n_count_meter,   aggregator_statistics_config, std::string, &aggregator_statistics_config::count_meter>
    >,
    json::strict_mode
  > meta;
  
  typedef meta::target target;
  typedef meta::serializer serializer;
  typedef meta::member_list member_list;
};

}}
