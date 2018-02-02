#pragma once

#include "core_config.hpp"

#include <wfc/core/workflow.hpp>
#include <wfc/domain_object.hpp>
#include <wfc/core/icore.hpp>
#include <vector>
#include <memory>

namespace wfc{  namespace core{

class core
  : public ::wfc::domain_object<icore, core_config, nostat>
  , public std::enable_shared_from_this<core>
{
public:
  virtual ~core();
  core();

  /// domain_object
  virtual void reconfigure() override;
  virtual void stop() override;
  
  /// icore
  virtual int run() override;
  virtual void core_reconfigure() override;
  virtual void core_stop( ) override;
  virtual void core_abort( std::string message ) override;
  
private:
  void _sunrise();
  bool _configure();
  void _initialize();
  void _start();
  void _stop();
  int  _main_loop();
  bool _idle();
  
private:
  std::atomic<bool> _reconfigure_flag;
  std::atomic<bool> _stop_flag;
  std::atomic<bool> _abort_flag;
  std::shared_ptr<core> _same;
  std::shared_ptr< workflow > _core_workflow;
};

}}
