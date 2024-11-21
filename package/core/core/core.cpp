#include "core.hpp"

#include <wfc/core/iconfig.hpp>
#include <wfc/module/iinstance.hpp>
#include <wfc/module/icomponent.hpp>
#include <wfc/core/cpuset.hpp>
#include <wfc/system/system.hpp>
#include <wfc/logger.hpp>
#include <wfc/memory.hpp>
#include <wfc/json.hpp>
#include <fas/system/nullptr.hpp>
#include <boost/filesystem.hpp>
#include <vector>
#include <string>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <syslog.h>
#include <sys/resource.h>
#include <sched.h>

#include "detail/status_log.hpp"

namespace wfc{  namespace core{



namespace
{
  static std::atomic<bool> gs_stop_signal;

  static void signal_sigint_handler(int sig)
  {
    // std::clog << "Stop signal handler" << std::endl;
    SYSTEM_LOG_WARNING("Stop signal handler " << sig)
    if ( auto g = ::wfc::wfcglobal::static_global )
      g->stop_signal_flag = true;
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
      boost::filesystem::path p{de};
      if ( boost::filesystem::is_directory(p) )
      {
        pid_t pid = ::atoi( p.filename().c_str() );
        if ( pid > 0 )
          pids.insert(pid);
      }
    });
    return pids;
  }

  std::string setaffinity(pid_t pid, const std::set<int>& cpu)
  {
    std::string ss = "[";
    cpu_set_t  mask;
    CPU_ZERO(&mask);

    for (int id : cpu )
    {
      ss += std::to_string(id) +  ",";
      CPU_SET( size_t(id), &mask);
    }
    if ( !cpu.empty() )
      ss.pop_back();
    ss+="]";
    if ( 0 != ::sched_setaffinity( pid, sizeof(cpu_set_t), &mask) )
    {
      SYSTEM_LOG_ERROR("sched_setaffinity: " << strerror(errno) << " for pid=" << pid);
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
  , _status_log(nullptr)
{
  gs_stop_signal = false;
}

void core::reconfigure()
{
  auto opt = this->options();
  auto cw_opt = opt.common_workflow;
  wflow::workflow_handlers cw_hndl;
  cw_opt.id = this->name();
  if ( auto g = this->global() )
  {
    _status_log = std::make_shared<status_log>(opt.status, &g->cpu);
    g->cpu.set_cpu( "common_workflow", opt.common_workflow.cpu);
    cw_hndl.startup_handler = []( std::thread::id ) noexcept
    {
      if ( auto gl = ::wfc::wfcglobal::static_global )
        gl->cpu.set_current_thread("common_workflow");
    };
    pid_t pid = 0;
    cw_hndl.status_handler = [pid, g]( std::thread::id ) mutable
    {
      pid = g->cpu.thread_active(pid);
    };
    cw_hndl.finish_handler = []( std::thread::id id)
    {
      if ( auto gl = ::wfc::wfcglobal::static_global )
      {
        SYSTEM_LOG_MESSAGE( "common_workflow thread finished. std::thread::id=" << id )
        gl->cpu.del_current_thread();
      }
    };

    if ( g->common_workflow==nullptr )
    {
      g->common_workflow = std::make_shared<wflow::workflow>( g->io_context, cw_opt, cw_hndl );
      g->common_workflow->start();
    }
    else
      g->common_workflow->reconfigure(cw_opt, cw_hndl);

    g->disable_statistics = opt.disable_statistics;

    bool nca = opt.nocall_callback_abort;
    bool ncs = opt.nocall_callback_show;
    bool dca = opt.double_callback_abort;
    bool dcs = opt.double_callback_show;

    g->nocall_handler = [g, nca, ncs](const std::string& objname)
    {
      if ( g->stop_signal_flag )
      {
        DOMAIN_LOG_WARNING("The callback handler for the '" << objname << "' was destroyed without a call after stop signal")
      }
      else if ( nca )
      {
        DOMAIN_LOG_FATAL("The callback handler for the '" << objname << "' was destroyed without a call")
      }
      else if ( ncs )
      {
        DOMAIN_LOG_ERROR("The callback handler for the '" << objname << "' was destroyed without a call")
      }
    };

    g->doublecall_handler = [g, dca, dcs](const std::string& objname)
    {
      if ( g->stop_signal_flag )
      {
        DOMAIN_LOG_WARNING("Double call callback functions for '" << objname << "' after stop signal")
      }
      else if ( dca )
      {
        DOMAIN_LOG_FATAL("Double call callback functions for '" << objname << "'")
      }
      else if ( dcs )
      {
        DOMAIN_LOG_ERROR("Double call callback functions for '" << objname << "'")
      }
    };
  }

  if ( opt.rlimit_as_mb != 0 )
  {
    rlim_t limit = opt.rlimit_as_mb*1024*1024;
    rlimit rlim = {RLIM_INFINITY, RLIM_INFINITY};
    if ( 0 == getrlimit( RLIMIT_AS, &rlim ) )
    {
      SYSTEM_LOG_MESSAGE("current RLIMIT_DATA: " << rlim.rlim_cur << ", " << rlim.rlim_max)
      SYSTEM_LOG_MESSAGE("rlimit_as_mb: " << opt.rlimit_as_mb << "Mb")

      rlim.rlim_cur = limit;
      if ( 0 != setrlimit( RLIMIT_AS, &rlim ) )
      {
        SYSTEM_LOG_ERROR("setrlimit: " << strerror(errno) )
      }
    }
    else
    {
      SYSTEM_LOG_ERROR("getrlimit: " << strerror(errno) )
    }
  }
  this->global()->cpu.set_current_thread( this->name() );

}

void core::stop()
{
  _same = this->shared_from_this();
  SYSTEM_LOG_MESSAGE("************* void core::stop()  *****************")
  _stop_flag = true;
}

int core::run()
{
  if ( this->global()->common_workflow==nullptr )
    this->reconfigure();

  gs_stop_signal = false;

  signal(SIGPIPE,  SIG_IGN);
  signal(SIGPOLL,  SIG_IGN);
  signal(SIGINT,   signal_sigint_handler);
  signal(SIGTERM,  signal_sigint_handler);

  SYSTEM_LOG_MESSAGE("core::run: sunrise!")

  this->_sunrise();

  if ( !_stop_flag )
  {
    SYSTEM_LOG_BEGIN("After start handlers... ")
    this->global()->after_start.fire();
    SYSTEM_LOG_END("After start handlers")
  }

  if ( !_stop_flag )
  {
    SYSTEM_LOG_MESSAGE("***************************************")
    SYSTEM_LOG_MESSAGE("************* started *****************")
    SYSTEM_LOG_MESSAGE("instance name: " << this->global()->instance_name << std::endl)
  }
  else if ( _abort_flag )
  {
    SYSTEM_LOG_WARNING("!!! START ABORTED! Смотрите выше.")
  }

  workflow_options qopt;
  qopt.use_asio = true;
  qopt.wrnsize = 0;
  qopt.maxsize = 0;
  qopt.threads = 0;

  workflow_handlers qwhs;
  _core_workflow = std::make_shared< wflow::workflow >( this->global()->io_context, qopt, qwhs );
  return this->_main_loop();
}

void core::core_reconfigure()
{
  _reconfigure_flag = true;
}

void core::core_stop()
{
  SYSTEM_LOG_MESSAGE("wfc_core: stop!")
  _stop_flag = true;
  if ( auto g = ::wfc::wfcglobal::static_global )
    g->stop_signal_flag = true;
}

void core::core_abort( const std::string& message )
{
  if ( auto g = ::wfc::wfcglobal::static_global )
  {
    if ( g->stop_signal_flag == false )
    {
      g->stop_signal_flag = true;
      SYSTEM_LOG_FATAL(message)
    }
  }
  gs_stop_signal = true;
  _stop_flag = true;
  _abort_flag = true;
}

void core::core_restart()
{
  SYSTEM_LOG_MESSAGE("wfc_core: restart!")
  _stop_flag = true;
  _restart_flag = true;
  if ( auto g = ::wfc::wfcglobal::static_global )
    g->stop_signal_flag = true;
}

void core::set_status(core_status s, const std::string& msg)
{
  _status_log->set_status(s, msg);
}

void core::set_stage(core_stage s)
{
  _status_log->set_stage(s);
}

core_status core::get_status(core_stage* stage, std::vector<std::pair<core_status, std::string>>* status_list)
{
  return _status_log->get_status(stage, status_list);
}

std::string core::get_status_text(size_t errlogs, size_t wrnlogs)
{
  return _status_log->get_status_text(errlogs, wrnlogs);
}

bool core::_idle()
{
  if ( gs_stop_signal )
  {
    SYSTEM_LOG_MESSAGE("wfc_core: stop signal")
    _stop_flag = true;
  }

  if ( _stop_flag )
  {
    SYSTEM_LOG_BEGIN("wfc_core: io_context stop...")
    this->global()->io_context.stop();
    SYSTEM_LOG_END("wfc_core: io_context stop done!")
    return false;
  }

  if ( _reconfigure_flag )
  {
    _reconfigure_flag = false;
    this->_sunrise();
    SYSTEM_LOG_MESSAGE("Daemon reconfigured!")
  }

  if ( !_stop_flag )
  {
    size_t cpucount = 0;
    ::wfc::cpuset& cpumgr = this->global()->cpu;
    _pid = cpumgr.thread_active(_pid);
    if ( cpumgr.clean_dirty() )
    {
      auto all_pids = ::wfc::core::get_threads();
      auto wfc_cpu = this->options().wfc_cpu;
      auto sys_cpu = this->options().sys_cpu;
      auto pids = cpumgr.get_pids();
      for ( pid_t p : pids )
      {
        all_pids.erase(p);
        auto cpu = cpumgr.get_cpu(p);
        if ( cpu.empty() )
          cpu = wfc_cpu;
        if ( cpu.empty() )
          continue;
        ++cpucount;
        std::string scpu = setaffinity( p, cpu );
        SYSTEM_LOG_MESSAGE("For WFC thread " << p << " ('" << cpumgr.get_name(p) << "') CPU set " << scpu )
      }

      if ( !sys_cpu.empty() )
      {
        for ( pid_t p : all_pids )
        {
          ++cpucount;
          std::string scpu = setaffinity( p, sys_cpu );
          SYSTEM_LOG_MESSAGE("For SYS thread " << p << " CPU set " << scpu )
        }
      }
      if ( cpucount > 0 )
      {
        SYSTEM_LOG_MESSAGE("CPU threads [" << cpucount << "] reconfigured. ProcID="
                            << ::getpid() << " ParentId=" << ::getppid() )
      }
    }
  }
  else
  {
    SYSTEM_LOG_WARNING("Stop signal after reconfigure. Probably an initialization error" )
    return this->_idle();
  }
  return true;
}

int core::_main_loop()
{
  this->global()->common_workflow->start();
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


  this->global()->io_context.run();
  this->global()->io_context.restart();
  this->_stop();
  return _restart_flag ? -1 : 0;
}

///
/// sunrise
///

void core::_sunrise()
{
  _status_log->set_stage(core_stage::CONFIGURING);
  SYSTEM_LOG_MESSAGE("----------- configuration -------------")
  this->_configure();

  if ( _stop_flag ) return;
  if ( !_abort_flag )
  {
    _status_log->set_stage(core_stage::INITIALIZING);
    SYSTEM_LOG_MESSAGE("----------- initialization ------------")
    this->_initialize();
  }

  if ( _stop_flag ) return;
  if ( !_abort_flag )
  {
    _status_log->set_stage(core_stage::STARTING);
    SYSTEM_LOG_MESSAGE("-------------- starting ---------------")
    this->_start();
  }

  if ( _stop_flag )
    return;

  if ( !_abort_flag )
  {
    _status_log->set_stage(core_stage::IS_RUN);
    WSYSLOG_INFO("daemon " << this->global()->program_name << " started!")
  }
  else
  {
    WSYSLOG_ALERT("daemon " << this->global()->program_name << " fail at the starting ")
  }
}

bool core::_configure()
{
  auto g = this->global();

  if ( g == nullptr)
    return true;

  if ( auto conf = g->registry.get_target<iconfig>("config") )
  {
    g->registry.for_each<icomponent>("component", [this, conf](const std::string& component_name, std::shared_ptr<icomponent> obj)
    {
      if ( this->_abort_flag )
      {
        SYSTEM_LOG_MESSAGE("Configure component '" << component_name << "' aborted!")
        return;
      }
      std::string confstr = conf->get_config(component_name);
      if ( !confstr.empty() )
      {
        //SYSTEM_LOG_BEGIN("Configure component '" << component_name << "'...")
        json::json_error er;
        if ( !obj->configure(confstr, &er ) )
        {
          auto message = json::strerror::message( er);
          auto trace = json::strerror::trace( er, confstr.begin(), confstr.end() );
          SYSTEM_LOG_ERROR(
            "Json unserialize error for component '" << component_name << "':"
            << message << ". " << std::endl << trace
          )
          this->_abort_flag = true;
        }

        if ( !this->_abort_flag ) { /*SYSTEM_LOG_END("Configure component '" << component_name << "'...Done")*/ }
        else { SYSTEM_LOG_END("Configure component '" << component_name << "'...aborted!") }
      }
      else
      {
        SYSTEM_LOG_MESSAGE("Configuration for '" << component_name << "' is not set")
      }
    });
  }
  else
  {
    SYSTEM_LOG_WARNING("Configure module is not set")
  }
  return true;
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
  bool dirty_flag = g->registry.dirty();
  g->registry.for_each<iinstance>("instance", [&instances, dirty_flag](const std::string& /*name*/, std::shared_ptr<iinstance> obj)
  {
    if ( dirty_flag || obj->is_reconfigured() )
      instances.push_back(obj);
  });

  if ( instances.empty() )
  {
    SYSTEM_LOG_MESSAGE("Initialization does not require. No changes to the registry objects.")
    return;
  }

  std::sort(instances.begin(), instances.end(), [](const instance_ptr& left, const instance_ptr& right)->bool
  {
    return left->startup_priority() < right->startup_priority();
  } );

  std::for_each(instances.begin(), instances.end(), [this,g](const instance_ptr& m)
  {
    if ( this->_abort_flag )
    {
      SYSTEM_LOG_WARNING("Initialize instance '" << m->name() << "' aborted!")
      return;
    }

    SYSTEM_LOG_BEGIN("Initialize instance '" << m->name() << "'... startup_priority="  << m->startup_priority() )
    m->initialize();

    if ( !this->_abort_flag ) { SYSTEM_LOG_END("Initialize instance '" << m->name() << "'...Done") }
    else { SYSTEM_LOG_WARNING("Initialize instance '" << m->name() << "'...aborted!") }

    g->io_context.poll();
    g->io_context.restart();
  });

  if ( auto d = g->registry.reset_dirty() )
  {
    SYSTEM_LOG_MESSAGE("Full initialization finished for " << d << " registry changed.")
  }
  else
  {
    SYSTEM_LOG_MESSAGE("Partial initialization finished for " << instances.size() << " reconfigured object.")
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
      SYSTEM_LOG_WARNING("Start instance '" << m->name() << "' aborted!")
      return;
    }

    SYSTEM_LOG_BEGIN("Start instance '" << m->name() << "'...")
    m->start(std::string());
    if ( !this->_abort_flag ) { SYSTEM_LOG_END("Start instance '" <<  m->name() << "'...Done") }
    else { SYSTEM_LOG_WARNING("Start instance '" <<  m->name() << "'...aborted!") }

    g->io_context.poll();
    g->io_context.restart();
  });
}

void core::_stop()
{
  auto g = this->global();

  if ( g == nullptr)
    return;

  _status_log->set_stage(core_stage::STOPPING);

  SYSTEM_LOG_BEGIN("stop '" << g->instance_name << "'...")

  SYSTEM_LOG_BEGIN("before stop handler")
  g->before_stop.fire();
  SYSTEM_LOG_END("before stop handler")

  SYSTEM_LOG_MESSAGE("----------- stopping... ---------------")

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
    SYSTEM_LOG_BEGIN("Stop instance '" << m->name() << "'...")
    if ( m->name()=="logger" && this->_abort_flag )
    {
      SYSTEM_LOG_WARNING("!!! WFC ABORTED! Смотрите выше.")
    }

    m->stop(std::string());
    SYSTEM_LOG_END("Stop instance '" <<  m->name() << "'...Done")
  });

  SYSTEM_LOG_BEGIN("After stop handlers run...")
  g->after_stop.fire();
  SYSTEM_LOG_END("After stop handlers ... Done")

  SYSTEM_LOG_BEGIN("Stop common workflow...")
  g->io_context.stop();
  g->common_workflow->stop();
  SYSTEM_LOG_END("Stop common workflow ... Done")

  if ( !_abort_flag )
  {
    WSYSLOG_INFO("daemon " << this->global()->program_name << " stopped!")
  }
  else
  {
    WSYSLOG_ALERT("daemon " << this->global()->program_name << "(" << this->global()->instance_name << ") Abnormal Shutdown!")
  }

  SYSTEM_LOG_END("Stop daemon '" << g->instance_name << "'...Done")
  _same = nullptr;
  SYSTEM_LOG_MESSAGE(std::endl)
}

}}
