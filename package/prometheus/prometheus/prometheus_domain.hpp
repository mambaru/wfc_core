//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2024
//
// Copyright: See COPYING file that comes with this distribution
//

#pragma once

#include <wfc/domain_object.hpp>
#include "prometheus_config.hpp"
#include <string>
#include <memory>

namespace wfc{ namespace core{

class prometheus_domain
  : public domain_object<iinterface, prometheus_config>
  , public std::enable_shared_from_this<prometheus_domain>
{
  class impl;
  typedef domain_object<iinterface, prometheus_config> self;
public:
  
  virtual ~prometheus_domain();
  virtual void configure() override;
  virtual void reconfigure() override;
  virtual void initialize() override;
  virtual void start() override;
  virtual void restart() override;
  virtual void stop() override;
private:
  std::shared_ptr<impl> _prometheus;
  timer_id_t _timer = -1;
};

}}
