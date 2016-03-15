//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2013-2015
//
// Copyright: See COPYING file that comes with this distribution
//


#pragma once

#include "startup_config.hpp"
#include <wfc/json.hpp>

namespace wfc{

struct startup_config_json
{
  typedef json::object<
    startup_config,
    fas::type_list_n<
    >::type
  > type;
  typedef type::serializer serializer;
  typedef type::member_list member_list;
  typedef type::target target;

};

}
