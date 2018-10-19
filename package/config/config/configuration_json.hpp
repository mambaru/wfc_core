//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2013-2018
//
// Copyright: See COPYING file that comes with this distribution
//

#pragma once

#include <wfc/json.hpp>

#include "configuration.hpp"

namespace wfc{  namespace core{

struct configuration_json
{
  typedef json::dict<
    std::unordered_map<
      json::value<std::string>,
      json::raw_value<std::string>
    >
  >::serializer serializer;
};

}}
