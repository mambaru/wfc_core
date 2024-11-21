//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2024
//
// Copyright: See COPYING file that comes with this distribution
//

#pragma once

#include "prometheus_config.hpp"
#include <wjson/wjson.hpp>

namespace wfc{ namespace core{

struct prometheus_config_json
{
  JSON_NAME(addr)
  JSON_NAME(port)
  JSON_NAME(update_ms)

  typedef wjson::object<
    prometheus_config,
    wjson::member_list<
      wjson::member< n_addr, prometheus_config, std::string, &prometheus_config::addr>,
      wjson::member< n_port, prometheus_config, std::string, &prometheus_config::port>,
      wjson::member< n_update_ms, prometheus_config, time_t, &prometheus_config::update_ms, wjson::time_interval_ms<time_t> >
    >,
    wjson::strict_mode
  > meta;

  typedef meta::target target;
  typedef meta::serializer serializer;
  typedef meta::member_list member_list;
};

}}
