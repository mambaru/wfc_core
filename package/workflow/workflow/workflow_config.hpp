#pragma once

#include <wfc/core/workflow_options.hpp>

namespace wfc{ namespace core{

struct workflow_statistics
{
  time_t interval_ms = 1000;
  std::string queue = ".queue";
  std::string dropped = ".dropped";
  std::string thread = ".thread";
};

struct workflow_config: workflow_options
{
  std::set<int> cpu;
//  statistics_params stat;
};

}}