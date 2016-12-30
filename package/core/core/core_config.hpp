#pragma once

#include <ctime>
#include <wfc/core/workflow_options.hpp>
#include <set>

namespace wfc{ namespace core{

struct core_config
{
  time_t core_timeout_ms = 1000;
  time_t idle_timeout_ms = 1000;
  size_t rlimit_as_mb = 0;
  std::set<int> cpu;
  std::set<int> unreg_cpu;
  bool enable_callback_check = true;
  workflow_options core_workflow;
  core_config() { /*core_workflow.enabled = true;*/ }
};

}}
