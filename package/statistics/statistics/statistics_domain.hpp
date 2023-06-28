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
: public domain_object<iinterface, statistics_config, statistics_statistics_config>
{
  class stat_impl;
public:
  virtual ~statistics_domain();
  virtual void reconfigure_basic() override;
  virtual void reconfigure() override;
  virtual void initialize() override;
  virtual void restart() override;
  virtual void stop() override;
  virtual void start() override;
  
private:
  typedef std::shared_ptr<stat_impl> stat_ptr;
  typedef std::weak_ptr<istatistics> statistics_wptr;

  stat_ptr get_stat_(const std::string& name);
  
  template<typename StatPtr>
  bool handler_(StatPtr st, size_t offset, size_t step);

  stat_ptr _stat;

  std::vector< statistics_wptr > _targets;
  timer_id_t _timer_id;
  std::string _log;
 
  std::atomic<bool> _started;
  std::chrono::system_clock::time_point _start_point;
  
  time_meter _push_meter;
  size_meter _count_meter;
  time_meter _multi_push_meter;
  size_meter _multi_count_meter;
  
  typedef rwlock<std::mutex> mutex_type;
  mutex_type _mutex;
  
  std::atomic<bool> _suspend_push;
};

}}
