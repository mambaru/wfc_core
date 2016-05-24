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

class logger_singleton_impl: public ::wfc::singleton<
  logger_singleton_name,
  ::wfc::instance<logger>,
  logger_config_json,
  component_features::CommonWorkflow
>
{  
};

logger_singleton::logger_singleton()
  : component( std::make_shared<logger_singleton_impl>() )
{
}

}
