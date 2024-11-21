//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2024
//
// Copyright: See COPYING file that comes with this distribution
//

#pragma once

#include <wfc/module/component.hpp>

namespace wfc{ namespace core{

class prometheus_multiton
  : public ::wfc::component
{
public:
  prometheus_multiton();
};

}}
