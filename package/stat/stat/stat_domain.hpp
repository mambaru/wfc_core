#pragma once

#include <wfc/domain_object.hpp>
#include <wfc/stat/istat.hpp>
#include <wfc/stat/ibtp.hpp>
#include "stat_config.hpp"
#include <string>
#include <memory>

namespace wfc{

class stat_domain
  : public domain_object<istat, stat_config>
{
  class impl;
public:
  virtual ~stat_domain();
  virtual void reconfigure() override;
  virtual void initialize() override;
  virtual       int reg_name(const std::string& name) override;
  virtual meter_ptr create_meter(int id, size_t count) override;
  virtual meter_ptr create_meter(const std::string& name, size_t count) override;
  virtual meter_ptr clone_meter(meter_ptr m, size_t count) override;

private:
  std::shared_ptr<impl> _impl;
  std::weak_ptr<ibtp> _wbtp;
  workflow_type::timer_id_t _stat_wf_id = -1;
  std::string _log;
};

}
