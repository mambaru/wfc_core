//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2013-2015
//
// Copyright: See COPYING file that comes with this distribution
//
#pragma once

#include "domain/logger_config.hpp"
#include <wfc/json.hpp>

namespace wfc{

struct logger_config_json
{
  JSON_NAME(single)
  JSON_NAME(limit)
  JSON_NAME(syslog)
  JSON_NAME(stdout)
  JSON_NAME(path)

  typedef json::object<
    logger_config,
    fas::type_list_n<
      json::member<n_single,   logger_config, bool,        &logger_config::single>,
      json::member<n_limit,    writer_config, size_t,      &writer_config::limit>,
      json::member<n_stdout,   writer_config, std::string, &writer_config::stdout>,
      json::member<n_path,     writer_config, std::string, &writer_config::path>,
      json::member<n_syslog,   writer_config, std::string, &writer_config::syslog>
    >::type
  > type;
  
  typedef type::serializer serializer;
  typedef type::target target;
  typedef type::member_list member_list;
  
};

}
