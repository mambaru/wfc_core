#pragma once

#include <wfc/domain_object.hpp>
#include <wfc/statistics/ibtp.hpp>
#include "statistics_config.hpp"
#include <string>
#include <memory>

namespace wfc{ namespace core{

class statistics_domain
  : public domain_object<iinterface, statistics_config>
{
  class impl;
public:
  virtual ~statistics_domain();
  virtual void reconfigure_basic() override;
  virtual void reconfigure() override;
  virtual void initialize() override;
  virtual void stop(const std::string&) override;
private:
  std::shared_ptr<impl> _impl;
  std::weak_ptr<ibtp> _wbtp;
  workflow_type::timer_id_t _stat_wf_id = -1;
  std::string _log;
};

}}
