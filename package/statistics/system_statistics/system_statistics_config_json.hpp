#pragma once

#include "system_statistics_config.hpp"
#include <wfc/json.hpp>
#include <wfc/statistics/stat_options_json.hpp>

namespace wfc{ namespace core{

struct system_statistics_config_json
{
  JSON_NAME(prefix)
  JSON_NAME(interval_ms)
  
  
  typedef json::object<
    system_statistics_config,
    json::member_list<
    >
  > type;
  
  typedef type::target target;
  typedef type::serializer serializer;
  typedef type::member_list member_list;
};

}}
