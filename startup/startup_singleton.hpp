//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2013-2015
//
// Copyright: See COPYING file that comes with this distribution
//

#pragma once

#include <wfc/module/component.hpp>

namespace wfc{

class startup_singleton_impl;

class startup_singleton
  : public ::wfc::component
{
public:
  startup_singleton();
};

}
