//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2024
//
// Copyright: See COPYING file that comes with this distribution
//

#pragma once

namespace wfc{ namespace core{

struct prometheus_config
{
  std::string addr="127.0.0.1";
  std::string port="8080";
  time_t update_ms = 1000;
};

}}
