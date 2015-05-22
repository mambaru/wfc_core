//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2013-2015
//
// Copyright: See COPYING file that comes with this distribution
//

#include "logger_object.hpp"
#include "domain/logger_domain.hpp"
#include "logger_config_json.hpp"

#include <wfc/module/singleton.hpp>
#include <wfc/module/instance.hpp>
#include <wfc/name.hpp>

namespace wfc{

WFC_NAME2(logger_object_name, "logger")

class logger_object_impl: public ::wfc::singleton<
  logger_object_name,
  ::wfc::instance<logger_domain>,
  logger_config_json
>
{  
};

logger_object::logger_object()
  : object( std::make_shared<logger_object_impl>() )
{
}

}
