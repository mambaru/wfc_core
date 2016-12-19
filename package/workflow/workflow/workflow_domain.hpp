#pragma once

#include <wfc/domain_object.hpp>
#include <wfc/core/workflow.hpp>
#include <wfc/statistics/meters.hpp>
#include "workflow_config.hpp"
#include <string>
#include <memory>

namespace wfc{ namespace core{

class workflow_domain
  : public domain_object<iinterface, workflow_config>
{
  class impl;
public:
  typedef std::shared_ptr< ::wfc::value_meter > value_meter_ptr;
  typedef ::wfc::workflow domain_interface;
  virtual ~workflow_domain();
  using domain_object<iinterface, workflow_config>::create;
  virtual void reconfigure() override;
  virtual void initialize() override;
  virtual void start() override;
  virtual void stop() override;
private:
  std::shared_ptr<impl> _workflow;
  timer_id_t _stat_timer;
  value_meter_ptr _meter_size;
  value_meter_ptr _meter_drop;
  std::vector<value_meter_ptr> _meters_threads;
  
  size_t _dropped = 0;
  std::vector<size_t> _counters;

};

}}
