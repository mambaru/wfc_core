//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2013-2015
//
// Copyright: See COPYING file that comes with this distribution
//

#pragma once

#include <string>

namespace wfc{

struct writer_config
{
  size_t limit = 0;
  std::string path = "";
  std::string stdout = "clog";
  std::string syslog = "";
  // запрещеные логи и типы
  std::vector< std::string > deny;
};

}
