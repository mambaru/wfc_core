//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2013-2018
//
// Copyright: See COPYING file that comes with this distribution
//

#pragma once

#include <ctime>
#include <wfc/core/workflow_options.hpp>
#include <package/core/core/detail/status_log_options.hpp>
#include <set>

namespace wfc{ namespace core{

struct common_workflow_options: workflow_options
{
  std::set<int> cpu;
};

struct core_config
{
  time_t core_timeout_ms = 1000;
  time_t idle_timeout_ms = 1000;
  size_t rlimit_as_mb = 0;
  bool disable_statistics = false;
  bool nocall_callback_abort = true;
  bool nocall_callback_show = true;
  bool double_callback_abort = true;
  bool double_callback_show = true;

  std::set<int> wfc_cpu;
  std::set<int> sys_cpu;

  status_log_options status;
  common_workflow_options common_workflow;
};

}}
