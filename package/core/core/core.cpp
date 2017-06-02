#include "core.hpp"

#include <wfc/core/iconfig.hpp>
#include <wfc/module/iinstance.hpp>
#include <wfc/module/icomponent.hpp>

#include <wfc/system/system.hpp>
#include <wfc/logger.hpp>
#include <wfc/memory.hpp>

#include <boost/filesystem.hpp>
#include <vector>
#include <string>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <syslog.h>
#include <sys/resource.h>
#include <sched.h>

namespace wfc{  namespace core{

namespace 
{
  static std::atomic<bool> gs_stop_signal;

  static void signal_sigint_handler(int)
  {
    std::clog << "Stop signal handler" << std::endl;
    gs_stop_signal = true;
  }
  
  std::set<pid_t> get_threads()
  {
    std::set<pid_t> pids;
    std::stringstream ss;
    ss << "/proc/" << ::getpid() << "/task";
    std::string dirname = ss.str();
    boost::system::error_code ec;
    boost::filesystem::directory_iterator beg( dirname, ec), end;
    if (ec)
      return pids;
  
    std::for_each(beg, end, [&pids](const boost::filesystem::directory_entry& de)
    {
      boost::filesystem::path p(de);
      if ( boost::filesystem::is_directory(p) )
      {
        pid_t pid = ::atoi( p.filename().c_str() );
        if ( pid > 0 )
          pids.insert(pid);
      }
    });
    return std::move(pids);
  }
  
  std::string setaffinity(pid_t pid, const std::set<int>& cpu)
  {
    std::string ss = "[";
    cpu_set_t  mask;
    CPU_ZERO(&mask);
    
    for (int id : cpu )
    {
      ss += std::to_string(id) +  ",";
      CPU_SET(id, &mask);
    }
    if ( !cpu.empty() )
      ss.pop_back();
    ss+="]";
    if ( 0 != ::sched_setaffinity( pid, sizeof(mask), &mask) )
    {
      DOMAIN_LOG_ERROR("sched_setaffinity: " << strerror(errno) << " for pid=" << pid);
    }
    return ss;
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

void core::create()
{
  this->global()->workflow->reconfigure(this->options().core_workflow);
  this->global()->workflow->start();
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
    DOMAIN_LOG_MESSAGE("!!! START ABORTED! Смотрите выше.")
  }

  ::iow::workflow_options qopt;
  qopt.use_io_service = true;
  qopt.wrnsize = 10;
  qopt.maxsize = 100;
  qopt.threads = 0;
  _core_workflow = std::make_shared< ::iow::workflow >( this->global()->io_service, qopt );
  return this->_main_loop();
}


void core::stop() 
{
  _same = this->shared_from_this();
  DOMAIN_LOG_MESSAGE("************* void core::stop()  *****************")
  _stop_flag = true;
}

void core::core_stop()
{
  DOMAIN_LOG_MESSAGE("wfc_core: stop!")
  _stop_flag = true;
}

void core::core_abort( std::string message ) 
{
  IOW_WRITE_LOG_FATAL("domain","")
  IOW_WRITE_LOG_FATAL("domain","!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!")
  IOW_WRITE_LOG_FATAL("domain","!!        АВАРИЙНОЕ ЗАВЕРШЕНИЕ")
  IOW_WRITE_LOG_FATAL("domain","!!        " << message )
  IOW_WRITE_LOG_FATAL("domain","!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!")
  IOW_WRITE_LOG_FATAL("domain","")
  _stop_flag = true;
  _abort_flag = true;
}

void core::start()
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
  this->global()->cpu.set_current_thread( this->name() );
}

bool core::_idle()
{
  if ( gs_stop_signal )
  {
    DOMAIN_LOG_MESSAGE("wfc_core: stop signal")
    _stop_flag = true;
  }

  if ( _stop_flag )
  {
    DOMAIN_LOG_BEGIN("wfc_core: io_service stop...")
    this->global()->io_service.stop();
    DOMAIN_LOG_END("wfc_core: io_service stop done!")
    return false;
  }

  if ( _reconfigure_flag )
  {
    _reconfigure_flag = false;
    this->_sunrise();
    DOMAIN_LOG_MESSAGE("Daemon reconfigured!")
  }

  if ( !_stop_flag )
  {
    DEBUG_LOG_MESSAGE("CPU thread check")
    ::wfc::cpuset& cpumgr = this->global()->cpu;
    if ( cpumgr.clean_dirty() )
    {
      DOMAIN_LOG_BEGIN("CPU threads reconfiguring...")
      auto all_pids = ::wfc::core::get_threads();
      auto wfc_cpu = this->options().cpu;
      auto sys_cpu = this->options().unreg_cpu;
      auto pids = cpumgr.get_pids();
      for ( pid_t p : pids )
      {
        all_pids.erase(p);
        auto cpu = cpumgr.get_cpu(p);
        if ( cpu.empty() )
          cpu = wfc_cpu;
        if ( cpu.empty() )
          continue;
        std::string scpu = setaffinity( p, cpu );
        COMMON_LOG_MESSAGE("For WFC thread " << p << " ('" << cpumgr.get_name(p) << "') CPU set " << scpu )
      }
      
      if ( !sys_cpu.empty() )
      {
        for ( pid_t p : all_pids )
        {
          std::string scpu = setaffinity( p, sys_cpu );
          COMMON_LOG_MESSAGE("For UNK thread " << p << " CPU set " << scpu )
        }
      }
      DOMAIN_LOG_END("CPU threads reconfigured.")
    }
  }
  return !_stop_flag;
}

int core::_main_loop()
{
  this->global()->workflow->start();
  std::weak_ptr<core> wthis = this->shared_from_this();

  _core_workflow->create_timer(
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
  
  _core_workflow->create_timer(
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
    SYSLOG_LOG_MESSAGE("daemon " << this->global()->program_name << " start abort....")    
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
        CONFIG_LOG_MESSAGE("Configure component '" << name << "' aborted!")
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

  if ( !g->registry.dirty() )
  {
    CONFIG_LOG_MESSAGE("Initialization does not require. No changes to the registry objects.")
    return;
  }

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
      CONFIG_LOG_MESSAGE("Initialize instance '" << m->name() << "' aborted!")
      return;
    }

    CONFIG_LOG_BEGIN("Initialize instance '" << m->name() << "'... startup_priority="  << m->startup_priority() )
    m->initialize();
 
    if ( !this->_abort_flag ) { CONFIG_LOG_END("Initialize instance '" << m->name() << "'...Done") }
    else { CONFIG_LOG_END("Initialize instance '" << m->name() << "'...aborted!") }

    g->io_service.poll();
    g->io_service.reset();
  });
  
  if ( auto d = g->registry.reset_dirty() )
  {
    CONFIG_LOG_MESSAGE("Initialization finished for " << d << " registry changed")
  }
  else
  {
    CONFIG_LOG_MESSAGE("Initialization finished. No registry changed")
  }
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
      CONFIG_LOG_MESSAGE("Start instance '" << m->name() << "' aborted!")
      return;
    }

    CONFIG_LOG_BEGIN("Start instance '" << m->name() << "'...")
    m->start(std::string());
    if ( !this->_abort_flag ) { CONFIG_LOG_END("Start instance '" <<  m->name() << "'...Done") }
    else { CONFIG_LOG_END("Start instance '" <<  m->name() << "'...aborted!") }
    
    g->io_service.poll();
    g->io_service.reset();
  });

  if ( auto d = g->registry.reset_dirty() )
  {
    CONFIG_LOG_MESSAGE(" ----------- DEBUG: Started finished for " << d << " registry changed")
  }
  else
  {
    CONFIG_LOG_MESSAGE("--------- DEBUG: Initialization finished. No registry changed")
  }

  auto opt = this->options();
  ///!!! this->global()->threads.set_reg_cpu( opt.cpu );
  ///!!! this->global()->threads.set_unreg_cpu( opt.unreg_cpu );
  // TODO: сделать по таймауту
  ///!!!  this->global()->threads.update_thread_list();
  /*
  if ( !opt.cpu.empty() )
  {
      std::vector<int> ids;
      ids.push_back(::getpid());
      this->global()->registry.for_each<workflow>("workflow", 
        [&ids](const std::string&, std::shared_ptr<workflow> wrk)
        {
          std::vector<int> cids = wrk->manager()->get_ids();
          std::copy( cids.begin(), cids.end(), std::back_inserter( ids) );
        }
      );

      for (int tid : ids)
      {
        cpu_set_t  mask;
        CPU_ZERO(&mask);
        for ( int cpu : opt.cpu )
        {
          CONFIG_LOG_MESSAGE("CPU_SET: " << cpu << " for " << tid )
          CPU_SET(cpu, &mask);
        }
        int result = ::sched_setaffinity( tid, sizeof(mask), &mask);
        if ( result == -1 )
        {
          CONFIG_LOG_ERROR("sched_setaffinity: " << strerror(errno) )
        }
      }
  }
  */

}

void core::_stop()
{
 
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
      DOMAIN_LOG_MESSAGE("!!! WFC ABORTED! Смотрите выше.")
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

}}
