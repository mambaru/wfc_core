#pragma once

#include <wfc/core/workflow_options_json.hpp>

namespace wfc{ namespace core{

struct workflow_config_json
{
  JSON_NAME(interval_ms)
  JSON_NAME(queue)
  JSON_NAME(dropped)
  JSON_NAME(thread)

  typedef workflow_config::stat_params stat_params;
  typedef json::object<
    stat_params,
    json::member_list<
      json::member<n_interval_ms, stat_params, time_t, &stat_params::interval_ms >,
      json::member<n_queue, stat_params, std::string, &stat_params::queue >,
      json::member<n_dropped, stat_params, std::string, &stat_params::dropped >,
      json::member<n_thread, stat_params, std::string, &stat_params::thread >
    >
  > stat_params_json;
  
  JSON_NAME(stat_params)
  typedef json::object<
    workflow_config,
    json::member_list<
      json::base< workflow_options2_json >,
      json::member<n_stat_params, workflow_config, stat_params, &workflow_config::stat, stat_params_json >
    >
  > type;
  typedef type::target target;
  typedef type::serializer serializer; 
  typedef type::member_list member_list; 
};

}}