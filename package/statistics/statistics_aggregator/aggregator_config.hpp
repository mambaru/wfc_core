//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2013-2018, 2023
//
// Copyright: See COPYING file that comes with this distribution
//

#pragma once


#include <wrtstat/multi_aggregator/multi_aggregator_options_json.hpp>
#include <wrtstat/multi_packer/packer_options.hpp>

namespace wfc{ namespace core{

struct packer_options:
  ::wrtstat::packer_options
{
  bool disabled = true;
};

struct aggregator_config:
  wrtstat::multi_aggregator_options
{
  // куда отправить входящие данные без массива data
  std::vector< std::string > targets;
  time_t pushout_timer_ms = 0;
  bool suspend_push = false;
  packer_options packer;
};

struct aggregator_statistics_config
{
  std::string multi_push_meter = "multi_push.time";
  std::string multi_count_meter = "multi_push.values";
  std::string push_meter = "push.time";
  std::string count_meter = "push.values";
};

}}
