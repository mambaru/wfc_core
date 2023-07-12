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
#include "aggregator_config.hpp"
#include <string>
#include <memory>
#include <atomic>

namespace wfc{ namespace core{

class aggregator_domain
: public domain_object<istatistics, aggregator_config, aggregator_statistics_config>
{
  class stat_impl;
public:
  virtual ~aggregator_domain();
  virtual void reconfigure() override;
  virtual void initialize() override;
  virtual void restart() override;
  virtual void stop() override;
  virtual void start() override;
  
  // istatistics
  virtual void push(push_ptr req, push_handler cb) override final;
  virtual void multi_push( multi_push_ptr req, multi_push_handler cb) override final;
  virtual void del( del_ptr req, del_handler cb) override final;
private:
  typedef std::shared_ptr<stat_impl> stat_ptr;
  typedef std::weak_ptr<istatistics> statistics_wptr;


  //stat_ptr get_stat_(const std::string& name);
  
/*  template<typename StatPtr>
  bool handler_(StatPtr st, size_t offset, size_t step);
*/
  void push_( push_ptr::element_type&& req);
  
  void push_next_( push_ptr req);

  void pack_next_mt_( push_ptr req);

  void mulit_push_next_( multi_push_ptr req);

  stat_ptr _stat;

  std::vector< statistics_wptr > _targets;
  timer_id_t _timer_id;
  std::string _log;
 
  /*std::atomic<bool> _started;
  std::chrono::system_clock::time_point _start_point;*/
  
  time_meter _push_meter;
  size_meter _count_meter;
  time_meter _multi_push_meter;
  size_meter _multi_count_meter;
  
  typedef rwlock<std::mutex> mutex_type;
  mutex_type _mutex;
  
  std::atomic<bool> _suspend_push;
  std::atomic<bool> _enabled_packer;
};

}}
