//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2013-2018, 2023
//
// Copyright: See COPYING file that comes with this distribution
//

#pragma once

#include "aggregator_config.hpp"
#include <wfc/json.hpp>
#include <wrtstat/multi_aggregator/multi_aggregator_options_json.hpp>
#include <wrtstat/multi_packer/packer_options_json.hpp>

namespace wfc{ namespace core{

struct packer_options_json
{
  JSON_NAME(disabled)
  typedef wjson::object<
    packer_options,
    wjson::member_list<
      json::member< n_disabled,   packer_options, bool, &packer_options::disabled>,
      json::base<wrtstat::packer_options_json>
     >,
    json::strict_mode
  > meta;

  typedef meta::target target;
  typedef meta::serializer serializer;
  typedef meta::member_list member_list;
};

struct aggregator_config_json
{
  JSON_NAME(targets)
  JSON_NAME(pushout_timer_ms)
  JSON_NAME(suspend_push)
  JSON_NAME(packer)

  
  typedef json::object<
    aggregator_config,
    json::member_list<
      json::base<wrtstat::multi_aggregator_options_json>,
      json::member< n_targets,  aggregator_config, std::vector<std::string>, &aggregator_config::targets,
                    json::vector_of_strings<> >,
      json::member< n_suspend_push,   aggregator_config, bool, &aggregator_config::suspend_push>,
      json::member< n_pushout_timer_ms, aggregator_config, time_t, &aggregator_config::pushout_timer_ms, wjson::time_interval_ms<> >,
      json::member< n_packer,  aggregator_config, packer_options, &aggregator_config::packer,
                    packer_options_json >
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
  JSON_NAME2(n_multi_count_meter, "multi_push.push_count")
  JSON_NAME2(n_push_meter, "push.time")
  JSON_NAME2(n_value_meter, "push.values")
  JSON_NAME(ag_counter)
  JSON_NAME(ag_data)
  JSON_NAME(packer_top)
  JSON_NAME(packer_data)

  typedef json::object<
    aggregator_statistics_config,
    json::member_list<
      json::member< n_multi_push_meter,   aggregator_statistics_config, std::string, &aggregator_statistics_config::multi_push_meter>,
      json::member< n_multi_count_meter,   aggregator_statistics_config, std::string, &aggregator_statistics_config::multi_count_meter>,
      json::member< n_push_meter,   aggregator_statistics_config, std::string, &aggregator_statistics_config::push_meter>,
      json::member< n_value_meter,   aggregator_statistics_config, std::string, &aggregator_statistics_config::value_meter>,
      json::member< n_ag_counter,   aggregator_statistics_config, std::string, &aggregator_statistics_config::ag_counter>,
      json::member< n_ag_data,   aggregator_statistics_config, std::string, &aggregator_statistics_config::ag_data>,
      json::member< n_packer_top,   aggregator_statistics_config, std::string, &aggregator_statistics_config::packer_top>,
      json::member< n_packer_data,   aggregator_statistics_config, std::string, &aggregator_statistics_config::packer_data>
    >,
    json::strict_mode
  > meta;
  
  typedef meta::target target;
  typedef meta::serializer serializer;
  typedef meta::member_list member_list;
};

}}
