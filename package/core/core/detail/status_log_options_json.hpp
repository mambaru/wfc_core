//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2024
//
// Copyright: See COPYING file that comes with this distribution
//

#pragma once

#include "status_log_options.hpp"
#include <wjson/wjson.hpp>

namespace wfc{ namespace core{

struct status_log_options_json
{
  JSON_NAME(hang_timeout_ms)
  JSON_NAME(errors_ttl_ms)
  JSON_NAME(warnings_ttl_ms)
  JSON_NAME(errors_limit)
  JSON_NAME(warnings_limit)

  typedef wjson::object<
    status_log_options,
    wjson::member_list<
       wjson::member<n_hang_timeout_ms, status_log_options, time_t, &status_log_options::hang_timeout_ms,
            wjson::time_interval_ms<time_t> >,
       wjson::member<n_errors_ttl_ms, status_log_options, time_t, &status_log_options::errors_ttl_ms,
            wjson::time_interval_ms<time_t> >,
       wjson::member<n_warnings_ttl_ms, status_log_options, time_t, &status_log_options::warnings_ttl_ms,
            wjson::time_interval_ms<time_t> >,
       wjson::member<n_errors_limit, status_log_options, size_t, &status_log_options::errors_limit >,
       wjson::member<n_warnings_limit, status_log_options, size_t, &status_log_options::warnings_limit >
    >,
    wjson::strict_mode
  > meta;

  typedef meta::target target;
  typedef meta::serializer serializer;
  typedef meta::member_list member_list;
};

}}

