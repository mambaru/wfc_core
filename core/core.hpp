#pragma once
#include <wfc/domain_object.hpp>
#include <wfc/module/iobject.hpp>
#include <wfc/core/icore.hpp>
#include <wfc/core/global.hpp>

#include <vector>
#include <utility>
#include <memory>

#include "core_config.hpp"

namespace wfc{

class core
  : public ::wfc::domain_object<icore, core_config>
{
public:

  virtual ~core();
  core();
  
  /// icore
  virtual int run();
  virtual void core_reconfigure();
  virtual void core_stop( );
  
  /// domain_object
  virtual void reconfigure();
  
private:
  
  void _sunrise();
  void _configure();
  void _initialize();
  void _start();
  void _stop();
  int _main_loop();
  void _idle();
  
private:

  typedef std::chrono::steady_clock::time_point time_point;
  typedef boost::asio::deadline_timer idle_timer;
  
  time_point _idle_time;
  std::atomic<bool> _reconfigure_flag;
  std::atomic<bool> _stop_flag;
  std::unique_ptr<idle_timer> _idle_timer;
};

}
