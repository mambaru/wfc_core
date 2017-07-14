#pragma once

#include <ctime>
#include <wfc/core/workflow_options.hpp>
#include <iow/io/aux/data_pool_options.hpp>
#include <set>

namespace wfc{ namespace core{

struct core_config
{
  time_t core_timeout_ms = 1000;
  time_t idle_timeout_ms = 1000;
  size_t rlimit_as_mb = 0;
  bool enable_callback_check = true;
  ::iow::io::data_map_options datapool;
  std::set<int> wfc_cpu;
  std::set<int> sys_cpu;
  workflow_options core_workflow;
};

}}
