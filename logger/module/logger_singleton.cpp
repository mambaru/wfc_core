//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2013-2015
//
// Copyright: See COPYING file that comes with this distribution
//

#include "logger_singleton.hpp"
#include "logger_config_json.hpp"
#include "../logger/logger.hpp"
#include <wfc/module/singleton.hpp>
#include <wfc/module/instance.hpp>
#include <wfc/name.hpp>

namespace wfc{

WFC_NAME2(logger_singleton_name, "logger")

class logger_singleton::impl: public ::wfc::singleton<
  logger_singleton_name,
  ::wfc::instance<logger>,
  logger_config_json
>
{  
};

logger_singleton::logger_singleton()
  : object( std::make_shared<logger_singleton::impl>() )
{
}

}
