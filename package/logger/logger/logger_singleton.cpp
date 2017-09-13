//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2013-2015
//
// Copyright: See COPYING file that comes with this distribution
//

#include "logger_singleton.hpp"
#include "logger_config_json.hpp"
#include "logger.hpp"
#include <wfc/module/singleton.hpp>
#include <wfc/module/instance.hpp>
#include <wfc/name.hpp>

namespace wfc{ namespace core{

namespace
{
  WFC_NAME2(singleton_name, "logger")

  class impl: public ::wfc::singleton
  <
    singleton_name,
    ::wfc::instance<logger>,
    logger_config_json,
      component_features::DisabledWorkflow
    | component_features::DisabledSuspend
  >
  {
  public:
    virtual std::string interface_name() const override
    {
      return std::string("wfc::ilogger");
    }

    virtual std::string description() const override
    {
      return std::string("Logging system");
    }
  };
}

logger_singleton::logger_singleton()
  : component( std::make_shared<impl>() )
{}

}}
