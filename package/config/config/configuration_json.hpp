#pragma once

#include <wfc/json.hpp>

#include "configuration.hpp"

namespace wfc{

struct configuration_json
{
  typedef json::dict<
    std::unordered_map<
      json::value<std::string>,
      json::raw_value<std::string>
    >
  >::serializer serializer;
};

}
