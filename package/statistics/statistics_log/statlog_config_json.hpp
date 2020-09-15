//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2013-2018
//
// Copyright: See COPYING file that comes with this distribution
//

#pragma once

#include "statlog_config.hpp"
#include <wfc/json.hpp>

namespace wfc{ namespace core{

struct table_format_options_json
{
  JSON_NAME(table_log)
  JSON_NAME(show_legend)
  JSON_NAME(sep)
  JSON_NAME(default_field_width)
  JSON_NAME(aliases)
  JSON_NAME(sequence_of_list)
  JSON_NAME(width_map)

  typedef json::object<
    table_format_options,
    json::member_list<

      json::member< n_table_log,  table_format_options, std::string, &table_format_options::table_log>,
      json::member< n_show_legend, table_format_options, bool,        &table_format_options::show_legend>,
      json::member< n_sep,  table_format_options, std::string, &table_format_options::sep>,
      json::member< n_default_field_width, table_format_options, int,        &table_format_options::default_field_width>,
      json::member< n_aliases, table_format_options, table_format_options::field_aliases_t,
                    &table_format_options::aliases, json::dict_map< json::value<std::string> > >,
      json::member< n_sequence_of_list, table_format_options, table_format_options::sequence_of_list_t,
                    &table_format_options::sequence_of_list, json::vector_of_strings<> >,
      json::member< n_width_map, table_format_options, table_format_options::width_map_t,
                    &table_format_options::width_map, json::dict_map< json::value<int> > >

     >
  > type;

  typedef type::target target;
  typedef type::serializer serializer;
  typedef type::member_list member_list;
};

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
  JSON_NAME(log_metric)
  JSON_NAME(table_format)

  typedef json::object<
    statlog_config,
    json::member_list<
      json::member< n_target,       statlog_config, std::string, &statlog_config::target>,
      json::member< n_common_log,   statlog_config, std::string, &statlog_config::common_log>,
      json::member< n_log_metric,   statlog_config, int,         &statlog_config::log_metric, enum_json>,
      json::member< n_table_format, statlog_config, table_format_options,
                    &statlog_config::table_format, table_format_options_json >
     >
  > type;

  typedef type::target target;
  typedef type::serializer serializer;
  typedef type::member_list member_list;
};

}}
