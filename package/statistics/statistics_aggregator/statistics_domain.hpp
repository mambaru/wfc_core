#pragma once

#include <wfc/domain_object.hpp>
#include <wfc/statistics/ibtp.hpp>
#include <wfc/statistics/stat_options.hpp>
#include "statistics_config.hpp"
#include <string>
#include <memory>

namespace wfc{ namespace core{

class statistics_domain
  : public domain_object<ibtp, statistics_config, ::wfc::stat_options>
{
  class impl;
public:
  virtual ~statistics_domain();
  virtual void reconfigure_basic() override;
  virtual void reconfigure() override;
  virtual void initialize() override;
  virtual void stop() override;
  virtual void add( wfc::btp::request::add::ptr req, wfc::btp::response::add::handler cb) override final;
private:
  std::shared_ptr<impl> _impl;
  std::weak_ptr<ibtp> _wbtp;
  workflow_type::timer_id_t _stat_wf_id = -1;
  std::string _log;
};

}}
