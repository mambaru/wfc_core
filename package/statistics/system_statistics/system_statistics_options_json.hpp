//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2013-2018
//
// Copyright: See COPYING file that comes with this distribution
//

#pragma once

#include "system_statistics_options.hpp"
#include <wfc/json.hpp>

namespace wfc{ namespace core{

struct system_statistics_options_json
{
  JSON_NAME(prefix)
  JSON_NAME(interval_ms)
  
  
  typedef json::object<
    system_statistics_options,
    json::member_list<
      json::member< n_prefix, system_statistics_options, std::string, &system_statistics_options::prefix>,
      json::member< n_interval_ms, system_statistics_options, time_t, &system_statistics_options::interval_ms>
    >
  > type;
  
  typedef type::target target;
  typedef type::serializer serializer;
  typedef type::member_list member_list;
};

}}
