#include "core.hpp"
#include <wfc/core/iconfig.hpp>
#include <wfc/system/system.hpp>
#include <wfc/module/iinstance.hpp>
#include <wfc/logger.hpp>
#include <wfc/memory.hpp>
#include <vector>
#include <string>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <syslog.h>
//#include <boost/asio.hpp>
#include <boost/asio/deadline_timer.hpp>



namespace wfc{

namespace {

static void signal_sigint_handler(int)
{
  std::clog << "Stop signal handler" << std::endl;
  if ( auto g = wfcglobal::static_global )
  {
    if ( auto c = g->registry.get<icore>("core") )
    {
      c->core_stop();
    }
  }
}


} // namespace


//typedef boost::asio::deadline_timer idle_timer;

core::~core()
{
  
}

core::core()
  : _reconfigure_flag(false)
  , _stop_flag(false)
{
}

void core::core_reconfigure()
{
  _reconfigure_flag = true;
}

int core::run( /*std::shared_ptr<wfcglobal> gl*/ )
{
  //_global = gl;
    
  signal(SIGPIPE,  SIG_IGN);
  signal(SIGPOLL,  SIG_IGN);
  signal(SIGINT,   signal_sigint_handler);
  signal(SIGTERM,  signal_sigint_handler);

  CONFIG_LOG_MESSAGE("core::run: sunrise!")
  
  this->_sunrise();  
  
  DOMAIN_LOG_BEGIN("after start handlers")
  this->global()->after_start.fire();
  DOMAIN_LOG_BEGIN("after start handlers")


  DOMAIN_LOG_MESSAGE("***************************************")
  DOMAIN_LOG_MESSAGE("************* started *****************")
  DOMAIN_LOG_MESSAGE("instance name: " << this->global()->instance_name << std::endl)
  
  return this->_main_loop();
}

void core::core_stop( )
{
  std::cout << "core stop" << std::endl;
  _stop_flag = true;
}

//void core::configure(const core_config& conf)
void core::reconfigure()
{
  auto opt = this->options();
  if ( opt.rlimit_as_mb != 0 )
  {
    rlim_t limit = opt.rlimit_as_mb*1024*1024;
    rlimit rlim = {RLIM_INFINITY, RLIM_INFINITY};
    if ( 0 == getrlimit( RLIMIT_AS, &rlim ) )
    {
      CONFIG_LOG_MESSAGE("current RLIMIT_DATA: " << rlim.rlim_cur << ", " << rlim.rlim_max)
      CONFIG_LOG_MESSAGE("rlimit_as_mb: " << opt.rlimit_as_mb << "Mb")
      
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
  // std::cout << "DEBUG IDLE" << std::endl;
  if ( _stop_flag )
  {
    DOMAIN_LOG_MESSAGE("wfc_core: stop signal")
    this->global()->io_service.stop();
    return;
  }
  
  this->global()->idle.fire();

  if ( _reconfigure_flag )
  {
    _reconfigure_flag = false;
    this->_sunrise();
    DOMAIN_LOG_MESSAGE("Daemon reconfigured!")
  }
  
  _idle_timer->expires_at(_idle_timer->expires_at() + boost::posix_time::milliseconds( this->options().idle_timeout_ms));
  _idle_timer->async_wait([this](const boost::system::error_code& /*e*/){
    this->_idle();  
  });
}

int core::_main_loop()
{
  _idle_timer = std::make_unique<idle_timer>(
    this->global()->io_service,  
    boost::posix_time::milliseconds(this->options().idle_timeout_ms) 
  );

  _idle_timer->async_wait([this](const boost::system::error_code& )
  {
    this->_idle();  
  });
  this->global()->io_service.run();
  this->global()->io_service.reset();
  
  this->_stop();
  return 0;
}


///
/// sunrise
///

void core::_prepare(module_vector& mv)
{
  this->global()->registry.for_each<iobject>("object",  [&mv](const std::string& name, std::shared_ptr<iobject> m){
    mv.push_back( module_pair( name, m) );
  });
}

void core::_sunrise()
{
  module_vector modules;
  /*this->_prepare(modules);
  std::sort(modules.begin(), modules.end(), [](const module_pair& left, const module_pair& right)->bool {
    return left.second->startup_priority() < right.second->startup_priority();
  } );
  */

  CONFIG_LOG_MESSAGE("----------- configuration -------------")
  this->_configure(modules);
  
  CONFIG_LOG_MESSAGE("----------- initialization ------------")
  this->_initialize(modules);
  
  CONFIG_LOG_MESSAGE("-------------- starting ---------------")
  this->_start(modules);
  
  SYSLOG_LOG_MESSAGE("daemon " << this->global()->program_name << " started!")
}

void core::_configure(const module_vector& modules)
{
  auto g = this->global();

  if ( g == nullptr)
    return;
  
  if ( auto conf = g->registry.get<iconfig>("config") )
  {
    g->registry.for_each<iobject>("object", [conf](const std::string& name, std::shared_ptr<iobject> obj)
    {
      std::string confstr = conf->get_config(name);
      if ( !confstr.empty() )
      {
        CONFIG_LOG_BEGIN("core::configure: object '" << name << "'...")
        obj->configure(confstr, std::string() );
        CONFIG_LOG_END("core::configure: object '" << name << "'...Done!")
      }
      else
      {
        CONFIG_LOG_ERROR("core::configure: configuration for '" << name << "' not found!")
      }
    });
  }
  else
  {
    DOMAIN_LOG_WARNING("Configure module is not set")
  }
  
  /*
  if (!_global)
    return;
  
  //if ( auto config = global->config )
  if ( auto config = _global->registry.get<iconfig>("config") )
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
    DOMAIN_LOG_WARNING("Configure module is not set")
  }
  */
}

void core::_initialize(const module_vector& modules)
{
  auto g = this->global();

  if ( g == nullptr)
    return;

  /*
  g->registry.for_each<iinstance>("instance", [g](const std::string& name, std::shared_ptr<iinstance> obj)
  {
    CONFIG_LOG_BEGIN("core::initialize: instance '" << name << "'...")
    obj->initialize();
    CONFIG_LOG_END("core::initialize: module '" << name << "'...Done!")
    g->io_service.poll();
    g->io_service.reset();
  });
  */

  
  typedef std::shared_ptr<iinstance> instance_ptr;
  typedef std::vector<instance_ptr> instance_list;
  instance_list instances;
  instances.reserve(100);
  g->registry.for_each<iinstance>("instance", [&instances](const std::string& name, std::shared_ptr<iinstance> obj)
  {
    instances.push_back(obj);
  });

  std::sort(instances.begin(), instances.end(), [](const instance_ptr& left, const instance_ptr& right)->bool {
    return left->startup_priority() < right->startup_priority();
  } );
  
  std::for_each(instances.begin(), instances.end(), [g](const instance_ptr& m)
  {
    CONFIG_LOG_BEGIN("core::initialize: instance '" << m->name() << "'...")
    m->initialize();
    CONFIG_LOG_END("core::initialize: module '" << m->name() << "'...Done!")
    g->io_service.poll();
    g->io_service.reset();
  });


  
  /*
  auto& io = _global->io_service;
  std::for_each(modules.begin(), modules.end(), [&io](const module_pair& m)
  {
    CONFIG_LOG_BEGIN("core::initialize: module '" << m.first << "'...")
    m.second->initialize(std::string());
    CONFIG_LOG_END("core::initialize: module '" << m.first << "'...Done!")
    io.poll();
    io.reset();
  });
  */
}

void core::_start(const module_vector& modules)
{
  auto g = this->global();

  if ( g == nullptr)
    return;
  
  typedef std::shared_ptr<iinstance> instance_ptr;
  typedef std::vector<instance_ptr> instance_list;
  instance_list instances;
  instances.reserve(100);
  g->registry.for_each<iinstance>("instance", [&instances](const std::string& name, std::shared_ptr<iinstance> obj)
  {
    instances.push_back(obj);
  });

  std::sort(instances.begin(), instances.end(), [](const instance_ptr& left, const instance_ptr& right)->bool {
    return left->startup_priority() < right->startup_priority();
  } );
  
  std::for_each(instances.begin(), instances.end(), [g](const instance_ptr& m)
  {
    CONFIG_LOG_BEGIN("core::start: module '" << m->name() << "'...")
    m->start(std::string());
    CONFIG_LOG_END("core::start: module '" <<  m->name() << "'...Done!")
    g->io_service.poll();
    g->io_service.reset();
  });

  /*
  auto& io = _global->io_service;
  std::for_each(modules.begin(), modules.end(), [&io](const module_pair& m)
  {
    CONFIG_LOG_BEGIN("core::start: module '" << m.first << "'...")
    m.second->start(std::string());
    CONFIG_LOG_END("core::start: module '" << m.first << "'...Done!")
    io.poll();
    io.reset();
  });
  */
}

void core::_stop()
{
  if ( _idle_timer!=nullptr )
    _idle_timer->cancel();
  
  auto g = this->global();

  if ( g == nullptr)
    return;

  DOMAIN_LOG_BEGIN("stop '" << g->instance_name << "'...")
  
  DOMAIN_LOG_BEGIN("before stop handler")
  g->before_stop.fire();
  DOMAIN_LOG_END("before stop handler")

  CONFIG_LOG_MESSAGE("----------- stopping... ---------------")

  typedef std::shared_ptr<iinstance> instance_ptr;
  typedef std::vector<instance_ptr> instance_list;
  instance_list instances;
  instances.reserve(100);

  g->registry.for_each<iinstance>("instance", [&instances](const std::string& name, std::shared_ptr<iinstance> obj)
  {
    instances.push_back(obj);
  });

  std::sort(instances.begin(), instances.end(), [](const instance_ptr& left, const instance_ptr& right)->bool {
    return left->shutdown_priority() < right->shutdown_priority();
  } );
  
  std::for_each(instances.begin(), instances.end(), [g](const instance_ptr& m)
  {
    CONFIG_LOG_BEGIN("core::stop: module '" << m->name() << "'...")
    m->stop(std::string());
    CONFIG_LOG_END("core::stop: module '" <<  m->name() << "'...Done!")
  });
  
  DOMAIN_LOG_BEGIN("after stop handlers")
  g->after_stop.fire();
  DOMAIN_LOG_END("after stop handlers")

  DOMAIN_LOG_END("stop '" << g->instance_name << "'...Done!")
  DOMAIN_LOG_MESSAGE("=======================================")


  /*
  module_vector modules;
  this->_prepare(modules);
  std::sort(modules.begin(), modules.end(), [](const module_pair& left, const module_pair& right)->bool {
    return left.second->shutdown_priority() > right.second->shutdown_priority();
  } );
  
  std::for_each(modules.begin(), modules.end(), [](module_pair& m)
  {
    CONFIG_LOG_BEGIN("core::stop: module '" << m.first << "'...")
    m.second->stop(std::string());
    CONFIG_LOG_END("core::stop: module '" << m.first << "'...Done!")
  });
  
  DOMAIN_LOG_BEGIN("after stop handlers")
  this->global()->after_stop.fire();
  DOMAIN_LOG_END("after stop handlers")

  DOMAIN_LOG_END("stop '" << _global->instance_name << "'...Done!")
  DOMAIN_LOG_MESSAGE("=======================================")
  */
}


}
