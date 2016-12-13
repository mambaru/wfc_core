#pragma once

#include <wfc/core/workflow_options.hpp>

namespace wfc{ namespace core{

struct workflow_config: workflow_options
{
  struct stat_names_t
  {
    std::string queue = ".queue";
    std::string dropped = ".dropped";
    std::string thread = ".thread";
  };

  stat_names_t stat_names;
};

}}