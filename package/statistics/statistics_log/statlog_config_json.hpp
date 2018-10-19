//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2013-2018
//
// Copyright: See COPYING file that comes with this distribution
//

#pragma once

#include "statlog_config.hpp"
#include <wfc/json.hpp>

namespace wfc{ namespace core{

struct statlog_config_json
{
  JSON_NAME(perseconds)
  JSON_NAME(seconds)
  JSON_NAME(milliseconds)
  JSON_NAME(microseconds)
  JSON_NAME(nanoseconds)
  JSON_NAME(hide)

  typedef json::enumerator<
    int,
    json::member_list<
      json::enum_value<n_hide,         int, -1>,
      json::enum_value<n_perseconds,   int, 0>,
      json::enum_value<n_nanoseconds,  int, 1>,
      json::enum_value<n_microseconds, int, 1000>,
      json::enum_value<n_milliseconds, int, 1000000>,
      json::enum_value<n_seconds,      int, 1000000000>
    >
  > enum_json;

  JSON_NAME(target)
  JSON_NAME(common_log)
  JSON_NAME(legend_log)
  JSON_NAME(table_log)
  JSON_NAME(log_metric)

  typedef json::object<
    statlog_config,
    json::member_list<
      json::member< n_target,     statlog_config, std::string, &statlog_config::target>,
      json::member< n_common_log,   statlog_config, std::string, &statlog_config::common_log>,
      json::member< n_legend_log,   statlog_config, std::string, &statlog_config::legend_log>,
      json::member< n_table_log,   statlog_config, std::string, &statlog_config::table_log>,
      json::member< n_log_metric, statlog_config, int,         &statlog_config::log_metric, enum_json>
     >
  > type;
  
  typedef type::target target;
  typedef type::serializer serializer;
  typedef type::member_list member_list;
};

}}
