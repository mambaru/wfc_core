#pragma once
#include <wfc/domain_object.hpp>
#include <wfc/module/icomponent.hpp>
#include <wfc/core/icore.hpp>
#include <wfc/core/global.hpp>
#include <wfc/core/workflow.hpp>

#include <vector>
#include <utility>
#include <memory>

#include "core_config.hpp"

namespace wfc{

class core
  : public ::wfc::domain_object<icore, core_config>
  , public std::enable_shared_from_this<core>
{
public:

  virtual ~core();
  core();
  
  /// icore
  virtual int run() override;
  virtual void core_reconfigure() override;
  virtual void core_stop( ) override;
  virtual void core_abort( std::string message ) override;
  
  /// domain_object
  virtual void reconfigure();
  virtual void stop( const std::string &) override;

  
private:
  
  void _sunrise();
  void _configure();
  void _initialize();
  void _start();
  void _stop();
  int  _main_loop();
  bool _idle();
  
private:

  /*
  typedef std::chrono::steady_clock::time_point time_point;
  typedef boost::asio::deadline_timer idle_timer;
  
  time_point _idle_time;
  std::unique_ptr<idle_timer> _idle_timer;
  */
  std::atomic<bool> _reconfigure_flag;
  std::atomic<bool> _stop_flag;
  std::atomic<bool> _abort_flag;
  std::shared_ptr<core> _same;
  std::shared_ptr<workflow> _core_timer;
};

}
