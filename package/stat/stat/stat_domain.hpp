#pragma once

#include <wfc/domain_object.hpp>
#include <wfc/statistics/ibtp.hpp>
#include "stat_config.hpp"
#include <string>
#include <memory>

namespace wfc{

class stat_domain
  : public domain_object<iinterface, stat_config>
{
  class impl;
public:
  virtual ~stat_domain();
  virtual void reconfigure_basic() override;
  virtual void reconfigure() override;
  virtual void initialize() override;
  virtual void stop(const std::string&) override;

  /*
  virtual meter_ptr create_meter(const std::string& rate_name, const std::string& size_name) override final;
  virtual meter_ptr clone_meter(meter_ptr m, size_t count ) override final;
  */


  /*
  virtual       int reg_name(const std::string& name) override;
  virtual meter_ptr create_meter(int id, size_t count) override;
  virtual meter_ptr create_meter(const std::string& name, size_t count) override;
  virtual meter_ptr clone_meter(meter_ptr m, size_t count) override;
  */

private:
  std::shared_ptr<impl> _impl;
  std::weak_ptr<ibtp> _wbtp;
  workflow_type::timer_id_t _stat_wf_id = -1;
  std::string _log;
};

}
