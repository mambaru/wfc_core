#pragma once

#include <wfc/module/module.hpp>

namespace wfc{

class logger_module_impl;

class logger_module
  : public ::wfc::module
{
public:
  logger_module();
};

}

