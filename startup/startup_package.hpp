//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2013-2015
//
// Copyright: See COPYING file that comes with this distribution
//

#pragma once

#include <wfc/module/package.hpp>

namespace wfc{

class startup_package_impl;

class startup_package
  : public ::wfc::package
{
public:
  startup_package();
};

}
