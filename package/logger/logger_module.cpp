//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2013-2016
//
// Copyright: See COPYING file that comes with this distribution
//

#include <wfc/module/component_list.hpp>
#include <wfc/name.hpp>

#include "logger_module.hpp"
#include "logger/logger_singleton.hpp"

namespace wfc{  namespace core{

namespace 
{
  WFC_NAME2(module_name, "logger")
  class impl: public ::wfc::component_list
  <
    module_name,
    logger_singleton
  >
  {
    virtual std::string description() const override
    {
      return "Оne-component module.";
    }
  };
}

logger_module::logger_module()
  : module( std::make_shared<impl>() )
{
}

}}
