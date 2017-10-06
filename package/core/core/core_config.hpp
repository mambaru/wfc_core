#pragma once

#include <ctime>
#include <wfc/core/workflow_options.hpp>
#include <iow/io/aux/data_pool_options.hpp>
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
  bool nocall_callback_abort = false;
  bool nocall_callback_show = true;
  bool double_callback_abort = false;
  bool double_callback_show = true;
  
  ::iow::io::data_map_options datapool;
  std::set<int> wfc_cpu;
  std::set<int> sys_cpu;
  common_workflow_options common_workflow;
};

}}
