#pragma once

#include <wfc/core/workflow_options.hpp>

namespace wfc{ namespace core{

struct workflow_config: workflow_options
{
  struct stat_params
  {
    time_t interval_ms = 1000;
    std::string queue = ".queue";
    std::string dropped = ".dropped";
    std::string thread = ".thread";
  };

  stat_params stat;
};

}}