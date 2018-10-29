//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2013-2018
//
// Copyright: See COPYING file that comes with this distribution
//

#pragma once

#include <wfc/domain_object.hpp>
#include "system_statistics_config.hpp"
#include "system_statistics_options.hpp"
#include <string>
#include <memory>

namespace wfc{  namespace core{

class system_statistics_domain
  : public domain_object<iinterface, system_statistics_config, system_statistics_options>
{
public:
  virtual void configure() override;
  virtual void restart() override;
  virtual void stop() override;
private:
  timer_id_t _timer_id = -1;
};

}}
