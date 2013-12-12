#pragma once

#include <wfc/json/json.hpp>
#include <wfc/json/name.hpp>
#include "configuration.hpp"
namespace wfc{

struct configuration_json
{
  typedef json::array<
    std::unordered_map<
      json::value<std::string>,
      json::raw_value<std::string>
    >
  >::serializer serializer;
};

}
