#pragma once
#include <wfc/core/imodule.hpp>
#include <wfc/core/icore.hpp>
#include <wfc/core/global.hpp>


#include <vector>
#include <utility>
#include <memory>

#include "core_config.hpp"

// TODO: move typedef to wfc
#include <boost/asio/deadline_timer.hpp>

namespace wfc{

//class idle_timer;
  
class core
  : public icore
{
  
public:

  virtual ~core();
  core();
  
  /// icore
  virtual int run( std::shared_ptr<global> g );
  virtual void reconfigure();
  virtual void stop( );

  // core_module
  void configure(const core_config& conf);
  
private:
  typedef std::pair<std::string, std::shared_ptr<imodule> > module_pair;
  typedef std::vector<module_pair> module_vector;

  void _prepare(module_vector& mv);
  void _sunrise();
  void _configure( const module_vector& m );
  void _initialize(const module_vector& m);
  void _start(const module_vector& m);
  void _stop();
  int _main_loop();

private:
  void _idle();
  
private:
  std::shared_ptr<global> _global;
  core_config _conf;

  typedef std::chrono::steady_clock::time_point time_point;
  time_point _idle_time;

  std::atomic<bool> _reconfigure_flag;
  std::atomic<bool> _stop_flag;
  typedef boost::asio::deadline_timer idle_timer;
  std::unique_ptr<idle_timer> _idle_timer;
  
};

}
