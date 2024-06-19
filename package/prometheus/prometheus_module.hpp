//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2024
//
// Copyright: See COPYING file that comes with this distribution
//

#pragma once

#include <wfc/module/module.hpp>

namespace wfc{ namespace core{

class prometheus_module
  : public ::wfc::module
{
public:
  prometheus_module();
};

}}
