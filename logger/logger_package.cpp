//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2013-2015
//
// Copyright: See COPYING file that comes with this distribution
//


#include "logger_build_info.h"
#include "logger_package.hpp"
#include "logger_module.hpp"
#include <wfc/module/module_list.hpp>
#include <wfc/name.hpp>

namespace wfc{

WFC_NAME2(logger_package_name, "logger")

class logger_package_impl: public ::wfc::module_list<
  logger_build_info,
  logger_module
>
{  
};

logger_package::logger_package()
  : package( std::make_shared<logger_package_impl>() )
{
}

}
