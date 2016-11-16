#pragma once

#include "stat_config.hpp"
#include <wfc/json.hpp>

namespace wfc{

struct stat_config_json
{
  typedef json::object<
    stat_config,
    json::member_list<
    >
  > type;
  
  typedef type::target target;
  typedef type::serializer serializer;
  typedef type::member_list member_list;
};

}
