//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2013-2015
//
// Copyright: See COPYING file that comes with this distribution
//

#pragma once

#include <wfc/module/object.hpp>

namespace wfc{

class logger_singleton
  : public ::wfc::object
{
  class impl;
public:
  logger_singleton();
};

}
