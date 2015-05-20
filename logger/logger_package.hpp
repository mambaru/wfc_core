#pragma once

#include <wfc/module/package.hpp>

namespace wfc{

class logger_package_impl;

class logger_package
  : public ::wfc::package
{
public:
  logger_package();
};

}
