#pragma once

#include <wfc/core/workflow_options_json.hpp>
#include "workflow_config.hpp"

namespace wfc{ namespace core{

struct workflow_config_json
{
  JSON_NAME(interval_ms)
  JSON_NAME(queue)
  JSON_NAME(dropped)
  JSON_NAME(thread)
  
  typedef workflow_config::statistics_params statistics_params;
  typedef json::object<
    statistics_params,
    json::member_list<
      json::member<n_interval_ms, statistics_params, time_t, &statistics_params::interval_ms >,
      json::member<n_queue, statistics_params, std::string, &statistics_params::queue >,
      json::member<n_dropped, statistics_params, std::string, &statistics_params::dropped >,
      json::member<n_thread, statistics_params, std::string, &statistics_params::thread >
    >
  > statistics_params_json;
  
  
  JSON_NAME(cpu)
  JSON_NAME(statistics_params)
  typedef json::object<
    workflow_config,
    json::member_list<
      json::base< workflow_options2_json >,
      json::member<n_cpu, workflow_config, std::set<int>, &workflow_config::cpu, json::array< std::set< json::value<int> > > >,
      json::member<n_statistics_params, workflow_config, statistics_params, &workflow_config::stat, statistics_params_json >
      
    >
  > type;
  typedef type::target target;
  typedef type::serializer serializer; 
  typedef type::member_list member_list; 
};

}}