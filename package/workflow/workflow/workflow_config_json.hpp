#pragma once

#include <wfc/core/workflow_options_json.hpp>

namespace wfc{

struct workflow_config_json
{
  JSON_NAME(queue)
  JSON_NAME(dropped)
  JSON_NAME(thread)

  typedef workflow_config::stat_names_t stat_names_t;
  typedef json::object<
    stat_names_t,
    json::member_list<
      json::member<n_queue, stat_names_t, std::string, &stat_names_t::queue >,
      json::member<n_dropped, stat_names_t, std::string, &stat_names_t::dropped >,
      json::member<n_thread, stat_names_t, std::string, &stat_names_t::thread >
    >
  > stat_names_json;
  
  JSON_NAME(stat_names)
  typedef json::object<
    workflow_config,
    json::member_list<
      json::base< workflow_options2_json >,
      json::member<n_stat_names, workflow_config, workflow_config::stat_names_t, &workflow_config::stat_names, stat_names_json >
    >
  > type;
  typedef type::target target;
  typedef type::serializer serializer; 
  typedef type::member_list member_list; 
};

}