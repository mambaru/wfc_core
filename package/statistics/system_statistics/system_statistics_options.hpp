//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2013-2018
//
// Copyright: See COPYING file that comes with this distribution
//

#pragma once

namespace wfc{ namespace core{

struct system_statistics_options
{
  std::string prefix;
  time_t interval_ms = 1000;
};

}}
