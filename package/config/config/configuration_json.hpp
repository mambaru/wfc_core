//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2013-2018
//
// Copyright: See COPYING file that comes with this distribution
//

#pragma once

#include <wjson/wjson.hpp>
#include "configuration.hpp"

namespace wfc{  namespace core{

struct configuration_json
{
  typedef wjson::dict_vector<wjson::raw_value<std::string> > meta;
  typedef meta::serializer serializer;
  typedef meta::target target;
};

}}
