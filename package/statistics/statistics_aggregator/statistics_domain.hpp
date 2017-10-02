#pragma once

#include <wfc/domain_object.hpp>
#include <wfc/statistics/istatistics.hpp>
#include "statistics_config.hpp"
#include <string>
#include <memory>
#include <atomic>

namespace wfc{ namespace core{

class statistics_domain
  : public domain_object<istatistics, statistics_config>
{
  class impl;
public:
  virtual ~statistics_domain();
  virtual void reconfigure_basic() override;
  virtual void reconfigure() override;
  virtual void initialize() override;
  virtual void stop() override;
  virtual void start() override;
  virtual void push( wfc::statistics::request::push::ptr req, wfc::statistics::response::push::handler cb) override final;
private:
  bool handler_(int offset, int step);

  std::shared_ptr<impl> _impl;
  typedef std::weak_ptr<istatistics> statistics_wptr;
  
  std::vector< statistics_wptr > _targets;
  statistics_wptr _target;

  workflow_type::timer_id_t _stat_wf_id = -1;
  std::string _log;
 
  std::atomic<bool> _started;
  std::chrono::system_clock::time_point _start_point;
};

}}
