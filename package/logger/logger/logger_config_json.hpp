//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2013-2015
//
// Copyright: See COPYING file that comes with this distribution
//
#pragma once

#include "logger_config.hpp"
#include <wfc/json.hpp>
#include <wlogjson/logger_options_json.hpp>

namespace wfc{ namespace core{

struct logger_config_json
{
  JSON_NAME(stop_with_fatal_log_entry)
  
  typedef json::object<
    logger_config,
    json::member_list<
      json::member<n_stop_with_fatal_log_entry,    logger_config, bool,      &logger_config::stop_with_fatal_log_entry>,
      json::base<wlog::logger_options_json>
    >,
    json::strict_mode
  > type;
  
  typedef type::serializer serializer;
  typedef type::target target;
  typedef type::member_list member_list;
};

  /*
struct logger_config_json
{
  JSON_NAME(limit)
  JSON_NAME(save_old)
  JSON_NAME(milliseconds)
  JSON_NAME(syslog)
  JSON_NAME(stdout)
  JSON_NAME(path)
  JSON_NAME(deny)

  typedef json::object<
    writer_options,
    json::member_list<
      json::member<n_milliseconds,    writer_options, bool,      &writer_options::milliseconds>,
      json::member<n_limit,    writer_options, size_t,      &writer_options::limit>,
      json::member<n_save_old,    writer_options, bool,      &writer_options::save_old>,
      json::member<n_stdout,   writer_options, std::string, &writer_options::stdout>,
      json::member<n_path,     writer_options, std::string, &writer_options::path>,
      json::member<n_syslog,   writer_options, std::string, &writer_options::syslog>,
      json::member<n_deny,   writer_options, std::vector<std::string>, &writer_options::deny,
        json::array<std::vector< json::value<std::string> > >
      >
    >,
    json::strict_mode
  > writer_options_json;
  
  JSON_NAME(single)
  JSON_NAME(custom)
  JSON_NAME(abort_with_fatal_message)
  
  typedef json::object<
    logger_config,
    json::member_list<
      json::base<writer_options_json>,
      json::member<n_single,   logger_config, bool,        &logger_config::single>,
      json::member<n_abort_with_fatal_message,    logger_config, bool,      &logger_config::abort_with_fatal_message>,
      json::member<n_custom,   logger_config, logger_config::custom_map, &logger_config::custom,
        json::dict< std::unordered_map< json::value<std::string>, writer_options_json > >
      >
    >,
    json::strict_mode
  > type;
  
  typedef type::serializer serializer;
  typedef type::target target;
  typedef type::member_list member_list;
};
  */

}}
