#pragma once

#include <comet/json/json.hpp>
#include <comet/json/name.hpp>
#include "configuration.hpp"
namespace mamba{ namespace comet{

struct configuration_json
{
  typedef json::array<
    std::unordered_map<
      json::value<std::string>,
      json::raw_value<std::string>
    >
  >::serializer serializer;
};

}}
