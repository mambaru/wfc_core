#pragma once

#include <wfc/domain_object.hpp>
#include <wfc/core/workflow.hpp>
#include "workflow_config.hpp"
#include <string>
#include <memory>

namespace wfc{

class workflow_domain
  : public domain_object<iinterface, workflow_config>
{
  class impl;
public:
  typedef ::wfc::workflow domain_interface;
  //typedef workflow_config options_type;
  virtual ~workflow_domain();
  using domain_object<iinterface, workflow_config>::create;
  virtual void reconfigure() override;
  virtual void initialize() override;
  virtual void start(const std::string& ) override;
  virtual void stop(const std::string& ) override;
private:
  std::shared_ptr<impl> _workflow;
  timer_id_t _stat_timer;
  meter_ptr _meter_size;
  meter_ptr _meter_drop;
  std::vector<meter_ptr> _meters_threads;
  
  size_t _dropped = 0;
  std::vector<size_t> _counters;

};

}
