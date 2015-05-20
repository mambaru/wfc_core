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
