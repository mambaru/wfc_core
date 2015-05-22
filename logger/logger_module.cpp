//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2013-2015
//
// Copyright: See COPYING file that comes with this distribution
//

#include "logger_module.hpp"
#include "logger_object.hpp"
#include <wfc/module/object_list.hpp>
#include <wfc/name.hpp>

namespace wfc{

WFC_NAME2(logger_module_name, "logger")

class logger_module_impl: public ::wfc::object_list<
  logger_module_name,
  logger_object
>
{
};

logger_module::logger_module()
  : module( std::make_shared<logger_module_impl>() )
{
}

}
