//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2013-2018
//
// Copyright: See COPYING file that comes with this distribution
//

#pragma once

#include <wfc/domain_object.hpp>
#include <wfc/statistics/istatistics.hpp>
#include <wfc/statistics/meters.hpp>
#include <wfc/mutex.hpp>
#include <wrtstat/wrtstat.hpp>
#include "statistics_config.hpp"
#include <string>
#include <memory>
#include <atomic>

namespace wfc{ namespace core{

class statistics_domain
  : public domain_object<istatistics, statistics_config, defstat>
{
  class stat_impl;
public:
  virtual ~statistics_domain();
  virtual void reconfigure_basic() override;
  virtual void reconfigure() override;
  virtual void initialize() override;
  virtual void ready() override;
  virtual void stop() override;
  virtual void start() override;
  virtual void push( wfc::statistics::request::push::ptr req, wfc::statistics::response::push::handler cb) override final;
  virtual void del( wfc::statistics::request::del::ptr req, wfc::statistics::response::del::handler cb) override final;
private:
  typedef std::shared_ptr<stat_impl> stat_ptr;
  typedef wrtstat::wrtstat stat_push;
  typedef std::shared_ptr<stat_push> stat_push_ptr;
  typedef std::vector<stat_push_ptr> stat_list;
  typedef std::vector<workflow_ptr> workflow_list;

  stat_ptr get_stat_(const std::string& name);
  //workflow_ptr get_workflow_(const std::string& name);
  
  template<typename StatPtr>
  bool handler_(StatPtr st, int offset, int step);

  stat_ptr _stat;
  stat_list _stat_list;
  workflow_list _workflow_list;
  
  typedef std::weak_ptr<istatistics> statistics_wptr;
  
  std::vector< statistics_wptr > _targets;
  //statistics_wptr _target;

  workflow_type::timer_id_t _timer_id;
  std::vector<workflow_type::timer_id_t> _timers; // удалить
  std::string _log;
 
  std::atomic<bool> _started;
  std::chrono::system_clock::time_point _start_point;
  
  time_factory _push_meter;
  size_factory _count_meter;
  
  typedef rwlock<std::mutex> mutex_type;
  mutex_type _mutex;
  
  std::atomic<bool> _suspend_push;
};

}}
