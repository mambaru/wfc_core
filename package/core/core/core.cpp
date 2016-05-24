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
#include <sys/resource.h>

namespace wfc{

namespace {

static std::atomic<bool> gs_stop_signal;

static void signal_sigint_handler(int)
{
  std::clog << "Stop signal handler" << std::endl;
  DOMAIN_LOG_MESSAGE("wfc_core: stop signal")
  gs_stop_signal = true;
}

} // namespace

core::~core()
{
}

core::core()
  : _reconfigure_flag(false)
  , _stop_flag(false)
  , _abort_flag(false)
{
  gs_stop_signal = false;
}

void core::core_reconfigure()
{
  _reconfigure_flag = true;
}

int core::run()
{
  gs_stop_signal = false;

  signal(SIGPIPE,  SIG_IGN);
  signal(SIGPOLL,  SIG_IGN);
  signal(SIGINT,   signal_sigint_handler);
  signal(SIGTERM,  signal_sigint_handler);

  CONFIG_LOG_MESSAGE("core::run: sunrise!")

  this->_sunrise();

  if ( !_stop_flag )
  {
    DOMAIN_LOG_BEGIN("After start handlers... ")
    this->global()->after_start.fire();
    DOMAIN_LOG_BEGIN("After start handlers")
  }

  if ( !_stop_flag )
  {
    DOMAIN_LOG_MESSAGE("***************************************")
    DOMAIN_LOG_MESSAGE("************* started *****************")
    DOMAIN_LOG_MESSAGE("instance name: " << this->global()->instance_name << std::endl)
  }
  else if ( _abort_flag )
  {
    DOMAIN_LOG_FATAL("!!! START ABORTED! Смотрите выше.")
  }

  ::iow::queue_options qopt;
  qopt.wrnsize = 10;
  qopt.maxsize = 100;
  _core_timer = ::wfc::workflow::create(this->global()->io_service, qopt);
  return this->_main_loop();
}


void core::stop( const std::string &) 
{
  _same = this->shared_from_this();
  DOMAIN_LOG_MESSAGE("************* void core::stop( const std::string &)  *****************")
  _stop_flag = true;
}

void core::core_stop( )
{
  DOMAIN_LOG_MESSAGE("wfc_core: stop!")
  _stop_flag = true;
}

void core::core_abort( std::string message ) 
{
  DOMAIN_LOG_FATAL("")
  DOMAIN_LOG_FATAL("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!")
  DOMAIN_LOG_FATAL("!!        АВАРИЙНОЕ ЗАВЕРШЕНИЕ")
  DOMAIN_LOG_FATAL("!!        " << message )
  DOMAIN_LOG_FATAL("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!")
  DOMAIN_LOG_FATAL("")
  _stop_flag = true;
  _abort_flag = true;
}

void core::reconfigure()
{
  auto opt = this->options();
  this->global()->workflow->reconfigure(opt.core_workflow);
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
}

bool core::_idle()
{
  if ( gs_stop_signal )
  {
    _stop_flag = true;
  }

  if ( _stop_flag )
  {
    DOMAIN_LOG_BEGIN("wfc_core: io_service stop...")
    this->global()->io_service.stop();
    DOMAIN_LOG_END("wfc_core: io_service stop done!")
    return false;
  }

  //this->global()->idle.fire();

  if ( _reconfigure_flag )
  {
    _reconfigure_flag = false;
    this->_sunrise();
    DOMAIN_LOG_MESSAGE("Daemon reconfigured!")
  }

  return !_stop_flag;
}

int core::_main_loop()
{
  this->global()->workflow->start();
  std::weak_ptr<core> wthis = this->shared_from_this();

  _core_timer->create_timer(
    std::chrono::milliseconds(this->options().idle_timeout_ms),
    [wthis]()->bool 
    {
      if (auto pthis = wthis.lock() )
      {
        pthis->global()->idle.fire();
        return pthis->_stop_flag==false;
      }
      return false;
    }
  );
  
  _core_timer->create_timer(
    std::chrono::milliseconds(this->options().core_timeout_ms),
    [wthis]()->bool 
    {
      if (auto pthis = wthis.lock() )
      {
        return pthis->_idle();
      }
      return false;
    }
  );
  
/*#warning сделать таймер и убрать wrk
  ::wfc::asio::io_service::work wrk(this->global()->io_service);*/
  this->global()->io_service.run();
  this->global()->io_service.reset();

  this->_stop();
  return 0;
}

///
/// sunrise
///

void core::_sunrise()
{
  CONFIG_LOG_MESSAGE("----------- configuration -------------")
  this->_configure();
  
  if ( _stop_flag ) return;
  if ( !_abort_flag )
  {
    CONFIG_LOG_MESSAGE("----------- initialization ------------")
    this->_initialize();
  }
  
  if ( _stop_flag ) return;
  if ( !_abort_flag )
  {
    CONFIG_LOG_MESSAGE("-------------- starting ---------------")
    this->_start();
  }
  
  if ( _stop_flag ) return;
  if ( !_abort_flag )
  {
    SYSLOG_LOG_MESSAGE("daemon " << this->global()->program_name << " started!")
  }
  else
  {
    SYSLOG_LOG_FATAL("daemon " << this->global()->program_name << " start abort")    
  }
}

void core::_configure()
{
  auto g = this->global();

  if ( g == nullptr)
    return;

  if ( auto conf = g->registry.get<iconfig>("config") )
  {
    g->registry.for_each<icomponent>("component", [this, conf](const std::string& name, std::shared_ptr<icomponent> obj)
    {
      if ( this->_abort_flag )
      {
        CONFIG_LOG_FATAL("Configure component '" << name << "' aborted!")
        return;
      }
      std::string confstr = conf->get_config(name);
      if ( !confstr.empty() )
      {
        CONFIG_LOG_BEGIN("Configure component '" << name << "'...")
        obj->configure(confstr, std::string() );
        
        if ( !this->_abort_flag ) { CONFIG_LOG_END("Configure component '" << name << "'...Done") }
        else { CONFIG_LOG_END("Configure component '" << name << "'...aborted!") }
      }
      else
      {
        CONFIG_LOG_MESSAGE("Configuration for '" << name << "' not set")
      }
    });
  }
  else
  {
    DOMAIN_LOG_WARNING("Configure module is not set")
  }
}

void core::_initialize()
{
  auto g = this->global();

  if ( g == nullptr)
    return;

  typedef std::shared_ptr<iinstance> instance_ptr;
  typedef std::vector<instance_ptr> instance_list;
  instance_list instances;
  instances.reserve(100);
  g->registry.for_each<iinstance>("instance", [&instances](const std::string& /*name*/, std::shared_ptr<iinstance> obj)
  {
    instances.push_back(obj);
  });

  std::sort(instances.begin(), instances.end(), [](const instance_ptr& left, const instance_ptr& right)->bool
  {
    return left->startup_priority() < right->startup_priority();
  } );

  std::for_each(instances.begin(), instances.end(), [this,g](const instance_ptr& m)
  {
    if ( this->_abort_flag )
    {
      CONFIG_LOG_FATAL("Initialize instance '" << m->name() << "' aborted!")
      return;
    }

    CONFIG_LOG_BEGIN("Initialize instance '" << m->name() << "'... startup_priority="  << m->startup_priority() )
    m->initialize();
 
    if ( !this->_abort_flag ) { CONFIG_LOG_END("Initialize module '" << m->name() << "'...Done") }
    else { CONFIG_LOG_END("Initialize module '" << m->name() << "'...aborted!") }

    g->io_service.poll();
    g->io_service.reset();
  });
}

void core::_start()
{
  auto g = this->global();

  if ( g == nullptr)
    return;

  typedef std::shared_ptr<iinstance> instance_ptr;
  typedef std::vector<instance_ptr> instance_list;
  instance_list instances;
  instances.reserve(100);
  g->registry.for_each<iinstance>("instance", [&instances](const std::string& /*name*/, std::shared_ptr<iinstance> obj)
  {
    instances.push_back(obj);
  });

  std::sort(instances.begin(), instances.end(), [](const instance_ptr& left, const instance_ptr& right)->bool
  {
    return left->startup_priority() < right->startup_priority();
  });

  std::for_each(instances.begin(), instances.end(), [this,g](const instance_ptr& m)
  {
    if ( this->_abort_flag )
    {
      CONFIG_LOG_FATAL("Start instance '" << m->name() << "' aborted!")
      return;
    }

    CONFIG_LOG_BEGIN("Start instance '" << m->name() << "'...")
    m->start(std::string());
    if ( !this->_abort_flag ) { CONFIG_LOG_END("Start instance '" <<  m->name() << "'...Done") }
    else { CONFIG_LOG_END("Start instance '" <<  m->name() << "'...aborted!") }
    
    //g->io_service.run_one();
    g->io_service.poll();
    g->io_service.reset();
  });
}

void core::_stop()
{
  /*if ( _idle_timer!=nullptr )
  {
    _idle_timer->cancel();
  }*/
  
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

  g->registry.for_each<iinstance>("instance", [&instances](const std::string& /*name*/, std::shared_ptr<iinstance> obj)
  {
    instances.push_back(obj);
  });

  std::sort(instances.begin(), instances.end(), [](const instance_ptr& left, const instance_ptr& right)->bool {
    return left->shutdown_priority() < right->shutdown_priority();
  } );

  std::for_each(instances.begin(), instances.end(), [this,g](const instance_ptr& m)
  {
    DOMAIN_LOG_BEGIN("Stop instance '" << m->name() << "'...")
    if ( m->name()=="logger" && this->_abort_flag )
    {
      DOMAIN_LOG_FATAL("!!! WFC ABORTED! Смотрите выше.")
    }

    m->stop(std::string());
    DOMAIN_LOG_END("Stop instance '" <<  m->name() << "'...Done")
  });

  CONFIG_LOG_BEGIN("After stop handlers run...")
  g->after_stop.fire();
  CONFIG_LOG_END("After stop handlers ... Done")

  DOMAIN_LOG_END("Stop daemon '" << g->instance_name << "'...Done")
  _same = nullptr;
  DOMAIN_LOG_MESSAGE("==================== Bye! ====================")
}

}
