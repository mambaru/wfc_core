#include "core.hpp"
#include "detail/po.hpp"
//#include <wfc/inet/epoller.hpp>
#include <wfc/core/global.hpp>
#include <wfc/module/imodule.hpp>
#include <wfc/core/iconfig.hpp>
#include <wfc/system/system.hpp>
#include <wfc/logger.hpp>
#include <wfc/memory.hpp>
#include <vector>
#include <string>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <syslog.h>
#include <boost/asio.hpp>


namespace wfc{

namespace {

static void signal_sigint_handler(int)
{
  std::cout << "Stop signal handler" << std::endl;
  if ( auto g = global::static_global )
  {
    //if ( auto c = g->core )
    if ( auto c = g->registry.get<icore>("core") )
    {
      c->stop();
    }
  }
}

} // namespace


core::~core()
{
  
}

core::core()
  : _reconfigure_flag(false)
  , _stop_flag(false)
{
  
}

void core::reconfigure()
{
  _reconfigure_flag = true;
}

int core::run( std::shared_ptr<global> gl )
{
  _global = gl;
  auto global = _global.lock();
    
  signal(SIGPIPE,  SIG_IGN);
  signal(SIGPOLL,  SIG_IGN);
  signal(SIGINT,   signal_sigint_handler);
  signal(SIGTERM,  signal_sigint_handler);

  CONFIG_LOG_MESSAGE("core::run: sunrise!")
  
  this->_sunrise();

  DAEMON_LOG_MESSAGE("***************************************")
  DAEMON_LOG_MESSAGE("************* started *****************")
  DAEMON_LOG_MESSAGE("instance name: " << global->instance_name << std::endl)
  
  return this->_main_loop();
}

void core::stop( )
{
  // Может вызываться из обработчика сигнала и получим deadlock в logger
  // TRACE_LOG_MESSAGE( "void core::stop( ) -1-" )
  _stop_flag = true;
}

void core::configure(const core_config& conf)
{
  this->_conf = conf;
  
  if ( this->_conf.rlimit_as_mb != 0 )
  {
    rlim_t limit = this->_conf.rlimit_as_mb*1024*1024;
    rlimit rlim = {RLIM_INFINITY, RLIM_INFINITY};
    if ( 0 == getrlimit( RLIMIT_AS, &rlim ) )
    {
      CONFIG_LOG_MESSAGE("current RLIMIT_DATA: " << rlim.rlim_cur << ", " << rlim.rlim_max)
      CONFIG_LOG_MESSAGE("rlimit_as_mb: " << this->_conf.rlimit_as_mb << "Mb")
      
      rlim.rlim_cur = limit;
      if ( 0 != setrlimit( RLIMIT_AS, &rlim ) )
      {
        CONFIG_LOG_ERROR("setrlimit: " << strerror(errno) )
      }
    }
    else
    {
      CONFIG_LOG_ERROR("getrlimit: " << strerror(errno) )
    }
    
  }
  CONFIG_LOG_MESSAGE("core module configured " )
}

void core::_idle()
{
  
  auto global = _global.lock();
  
  if ( _stop_flag )
  {
    DAEMON_LOG_MESSAGE("wfc_core: stop signal")
    global->io_service.stop();
    return;
  }
  
  global->idle.fire([](global::idle_handler handler){ return handler();});

  if ( _reconfigure_flag )
  {
    _reconfigure_flag = false;
    this->_sunrise();
    DAEMON_LOG_MESSAGE("Daemon reconfigured!")
  }
  
  _idle_timer->expires_at(_idle_timer->expires_at() + boost::posix_time::milliseconds(_conf.idle_timeout_ms));
  _idle_timer->async_wait([this](const boost::system::error_code& /*e*/){
    this->_idle();  
  });
}

int core::_main_loop()
//try
{
  if ( auto g = _global.lock() )
  {
    _idle_timer = std::make_unique<idle_timer>(g->io_service,  boost::posix_time::milliseconds(_conf.idle_timeout_ms) );
    _idle_timer->async_wait([this](const boost::system::error_code& ){
      this->_idle();  
    });
    g->io_service.run();
    g->io_service.reset();
  }
  
  this->_stop();
  
  /*
  if ( auto g = _global.lock() )
  {
    while ( 0!=g->io_service.poll() )
    {
     
    }
  }
  */
  
  return 0;
  
}
/*
catch(const std::exception& e)
{
  DAEMON_LOG_MESSAGE("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!")
  if ( auto global = _global.lock() )
  {
    DAEMON_LOG_FATAL( global->instance_name << ": " << e.what() )
  }
  throw;
}
catch(...)
{
  auto global = _global.lock();
  DAEMON_LOG_MESSAGE("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!")
  if ( auto global = _global.lock() )
  {
    DAEMON_LOG_FATAL( global->instance_name << ": unhandled exception" )
  }
  throw;
}
*/

///
/// sunrise
///

void core::_prepare(module_vector& mv)
{
  if ( auto global = _global.lock() )
  {
    /*if (auto gm = global->modules )
    {*/
      global->registry.for_each<imodule>([&mv](const std::string& /*prefix*/, const std::string& name, std::shared_ptr<imodule> m){
        mv.push_back( module_pair( name, m) );
      });
    //}
  }
}

void core::_sunrise()
{
  module_vector modules;
  this->_prepare(modules);
  std::sort(modules.begin(), modules.end(), [](const module_pair& left, const module_pair& right)->bool {
    return left.second->startup_priority() < right.second->startup_priority();
  } );

  CONFIG_LOG_MESSAGE("----------- configuration -------------")
  this->_configure(modules);
  
  CONFIG_LOG_MESSAGE("----------- initialization ------------")
  this->_initialize(modules);
  
  CONFIG_LOG_MESSAGE("-------------- starting ---------------")
  this->_start(modules);
}

void core::_configure(const module_vector& modules)
{
  auto global = _global.lock();
  if ( !global )
    return;
  
  //if ( auto config = global->config )
  if ( auto config = global->registry.get<iconfig>("config") )
  {
    std::for_each(modules.begin(), modules.end(), [config](const module_pair& m)
    {
      std::string confstr = config->get_config(m.first);
      if ( !confstr.empty() )
      {
        CONFIG_LOG_BEGIN("core::configure: module '" << m.first << "'...")
        m.second->configure(confstr, std::string() );
        CONFIG_LOG_END("core::configure: module '" << m.first << "'...Done!")
      }
      else
      {
        CONFIG_LOG_ERROR("core::configure: configuration for '" << m.first << "' not found!")
      }
    });
  }
  else
  {
    DAEMON_LOG_WARNING("Configure module is not set")
  }
}

void core::_initialize(const module_vector& modules)
{
  auto& io = _global.lock()->io_service;
  std::for_each(modules.begin(), modules.end(), [&io](const module_pair& m)
  {
    CONFIG_LOG_BEGIN("core::initialize: module '" << m.first << "'...")
    m.second->initialize(std::string());
    CONFIG_LOG_END("core::initialize: module '" << m.first << "'...Done!")
    io.poll();
    io.reset();
  });
}

void core::_start(const module_vector& modules)
{
  auto& io = _global.lock()->io_service;
  std::for_each(modules.begin(), modules.end(), [&io](const module_pair& m)
  {
    CONFIG_LOG_BEGIN("core::start: module '" << m.first << "'...")
    m.second->start(std::string());
    CONFIG_LOG_END("core::start: module '" << m.first << "'...Done!")
    io.poll();
    io.reset();
  });
}

void core::_stop()
{
  if ( _idle_timer!=nullptr )
    _idle_timer->cancel();
  
  auto global = _global.lock();
  if ( !global )
    return;

  DAEMON_LOG_BEGIN("stop '" << global->instance_name << "'...")
  CONFIG_LOG_MESSAGE("----------- stopping... ---------------")
  module_vector modules;
  this->_prepare(modules);
  std::sort(modules.begin(), modules.end(), [](const module_pair& left, const module_pair& right)->bool {
    return left.second->shutdown_priority() > right.second->shutdown_priority();
  } );

  /*
  if (auto gm = global->modules.lock() )
    gm->clear();
  */
  
  std::for_each(modules.begin(), modules.end(), [](module_pair& m)
  {
    CONFIG_LOG_BEGIN("core::stop: module '" << m.first << "'...")
    m.second->stop(std::string());
    
    //m.second.reset();
    CONFIG_LOG_END("core::stop: module '" << m.first << "'...Done!")
  });
  
  DAEMON_LOG_END("stop '" << global->instance_name << "'...Done!")
  DAEMON_LOG_MESSAGE("=======================================")

}


}
