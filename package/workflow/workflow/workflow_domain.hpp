//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2013-2018
//
// Copyright: See COPYING file that comes with this distribution
//

#pragma once

#include <wfc/domain_object.hpp>
#include <wfc/core/workflow.hpp>
#include <wfc/statistics/meters.hpp>
#include "workflow_config.hpp"
#include <string>
#include <memory>

namespace wfc{ namespace core{

class workflow_domain
  : public domain_object<iinterface, workflow_config, workflow_statistics>
  , public std::enable_shared_from_this<workflow_domain>
{
  class impl;
  typedef domain_object<iinterface, workflow_config, workflow_statistics> self;
public:
  typedef workflow_type::timer_id_t timer_id_t;
  
  virtual ~workflow_domain();
  virtual void configure() override;
  virtual void reconfigure() override;
  virtual void initialize() override;
  virtual void start() override;
  virtual void restart() override;
  virtual void stop() override;
private:
  wflow::workflow_handlers _handlers;
  std::shared_ptr<impl> _workflow;
  timer_id_t _stat_timer;
  value_meter _meter_size;
  value_meter _meter_drop;
  size_t _dropped = 0;
};

}}
