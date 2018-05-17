#pragma once

#include <wfc/core/workflow_options_json.hpp>
#include "workflow_config.hpp"

namespace wfc{ namespace core{
  
struct workflow_statistics_json
{
  JSON_NAME(interval_ms)
  JSON_NAME(queue)
  JSON_NAME(dropped)
  JSON_NAME(thread)
  
  typedef json::object<
    workflow_statistics,
    json::member_list<
      json::member<n_interval_ms, workflow_statistics, time_t, &workflow_statistics::interval_ms >,
      json::member<n_queue, workflow_statistics, std::string, &workflow_statistics::queue >,
      json::member<n_dropped, workflow_statistics, std::string, &workflow_statistics::dropped >,
      json::member<n_thread, workflow_statistics, std::string, &workflow_statistics::thread >
    >,
    ::wjson::strict_mode

  > type;

  typedef type::target target;
  typedef type::serializer serializer; 
  typedef type::member_list member_list; 
};

  
struct workflow_config_json
{
  JSON_NAME(cpu)
  typedef json::object<
    workflow_config,
    json::member_list<
      json::base< workflow_options_json >
    >,
    ::wjson::strict_mode
  > type;
  typedef type::target target;
  typedef type::serializer serializer; 
  typedef type::member_list member_list; 
};

}}
