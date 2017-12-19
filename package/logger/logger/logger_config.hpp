//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2013-2015
//
// Copyright: See COPYING file that comes with this distribution
//

#pragma once

#include <wlog/logger/logger_options.hpp>
#include <string>


namespace wfc{ namespace core{

struct logger_config: wlog::logger_options
{
  bool stop_with_fatal_log_entry = true;
};

}}
