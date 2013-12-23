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

// using idle_timer = ::boost::asio::deadline_timer;
  
  /*
class idle_timer
  : public boost::asio::deadline_timer
{
  //boost::asio::deadline_timer timer;
};
*/
  
  
namespace {

static void signal_sigint_handler(int)
{
  if ( auto g = global::static_global.lock() )
    if ( auto c = g->core.lock() )
      c->stop();
  exit(0);
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

int core::run( /*int argc, char* argv[],*/ std::weak_ptr<global> gl )
{
  _global = gl;
  auto global = _global.lock();
  _io_service = std::make_shared<wfc::io_service>();
  global->io_service = this->_io_service;

  /*if ( !this->_startup(argc, argv) )
      return 0;
      */
  
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
  _stop_flag = true;
}

void core::configure(const core_config& conf)
{
  CONFIG_LOG_MESSAGE("core module configured")
  this->_conf = conf;
}

void core::_idle()
{
  if ( _stop_flag )
  {
    this->_io_service->stop();
    return;
  }

  auto global = _global.lock();
  
  global->idle.fire([](global::idle_callback callback){ return callback();});

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
try
{
  _idle_timer = std::make_unique<idle_timer>(*_io_service,  boost::posix_time::milliseconds(_conf.idle_timeout_ms) );
  _idle_timer->async_wait([this](const boost::system::error_code& /*e*/){
    this->_idle();  
  });
  /*
  this->_io_service->dispatch([this](){
    if ( _stop_flag )
    {
      this->_io_service->stop();
      return;
    }
    
    if (_reconfigure_flag)
    {
      _reconfigure_flag = false;
      this->_sunrise();
      DAEMON_LOG_MESSAGE("Daemon reconfigured!")
    }
  });*/
  
  //for(;!_stop_flag;)
  {
    std::cout << "poll " << std::endl;
    this->_io_service->run();
  }
  this->_stop();
  /*
  _idle_time = std::chrono::steady_clock::now();
  //std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(_idle_time.time_since_epoch() ).count() << std::endl;
  for(;!_stop_flag;)
  {
    if ( _reconfigure_flag )
    {
      _reconfigure_flag = false;
      this->_sunrise();
      DAEMON_LOG_MESSAGE("Daemon reconfigured!")
    }
    
    this->_mux->select(_conf.wait_timeout_ms);
    auto now = std::chrono::steady_clock::now();
    if ( _idle_time < now  )
    {
      _global->idle.fire([](global::idle_callback callback){ return callback();});
      _idle_time += std::chrono::milliseconds(_conf.idle_timeout_ms);
    }
  }

  this->_stop();
  */
  return 0;
}
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

/*
bool core::_startup(int argc, char** argv)
{
  detail::po p = detail::po::parse(argc, argv);

  this->_global->program_name = p.program_name;
  this->_global->instance_name = p.instance_name.empty()
                                 ? p.instance_name
                                 : p.program_name;
  
  if (p.usage)
    this->_show_usage();
  
  if (p.help)
    this->_show_help();

  if (p.info)
  {
    if ( p.info_name.empty() )
      this->_show_info();
    this->_show_module_info(p.info_name);
  }

  if (p.generate)
    this->_generate(p.generate_name, p.config_path);

  if ( p.usage || p.help || p.info || p.generate )
    return false;

  if ( auto c = this->_global->config.lock() )
    c->initialize(p.config_path);

  if ( p.daemonize )
    ::wfc::daemonize();

  if ( p.daemonize && p.autoup )
  {
    ::wfc::autoup(p.autoup_timeout, [p](bool restart, int status)
    {
      ::openlog( (p.instance_name+"(wfc_core)").c_str(), 0, LOG_USER);
      std::stringstream ss;
      ss << "Daemon stop with status: " << status << ". ";
      if ( restart )
        ss << "Restarting...";
      else
        ss << "Do not restarted.";
      ::syslog(LOG_ERR, ss.str().c_str());
      ::closelog();
    });
  }

  if ( p.coredump )
    ::wfc::dumpable();
 
  return true;
}
*/

///
/// sunrise
///

void core::_prepare(module_vector& mv)
{
  if ( auto global = _global.lock() )
  {
    if (auto gm = global->modules.lock() )
    {
      gm->for_each([&mv](const std::string& name, std::weak_ptr<imodule> m){
        mv.push_back( module_pair( name, m.lock()) );
      });
    }
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
  
  auto config = global->config.lock();
  if ( config )
  {
    std::for_each(modules.begin(), modules.end(), [config](const module_pair& m)
    {
      std::string confstr = config->get_config(m.first);
      if ( !confstr.empty() )
      {
        CONFIG_LOG_BEGIN("core::configure: module '" << m.first << "'...")
        m.second->configure(confstr);
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
  std::for_each(modules.begin(), modules.end(), [](const module_pair& m)
  {
    CONFIG_LOG_BEGIN("core::initialize: module '" << m.first << "'...")
    m.second->initialize();
    CONFIG_LOG_END("core::initialize: module '" << m.first << "'...Done!")
  });
}

void core::_start(const module_vector& modules)
{
  std::for_each(modules.begin(), modules.end(), [](const module_pair& m)
  {
    CONFIG_LOG_BEGIN("core::start: module '" << m.first << "'...")
    m.second->start();
    CONFIG_LOG_END("core::start: module '" << m.first << "'...Done!")
  });
}

void core::_stop()
{
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

  //this->_stop(modules);
  DAEMON_LOG_END("stop '" << global->instance_name << "'...Done!")
  DAEMON_LOG_MESSAGE("=======================================")

  std::for_each(modules.begin(), modules.end(), [](const module_pair& m)
  {
    CONFIG_LOG_BEGIN("core::start: module '" << m.first << "'...")
    m.second->stop();
    CONFIG_LOG_END("core::stop: module '" << m.first << "'...Done!")
  });
}

///
/// help
///

/*
void core::_show_usage()
{
  std::cout <<  "Usage" << std::endl;
  std::cout <<  "  " << _global->program_name << " -h" << std::endl;
  std::cout <<  "  " << _global->program_name << " [-h] -i [<module name>]" << std::endl;
  std::cout <<  "  " << _global->program_name << " -G [<generate name>] [-C <path>]" << std::endl;
  std::cout <<  "  " << _global->program_name << " [-d] [-c] [-a <timeout>] [-n <instance name>] -C <path>" << std::endl;
}


void core::_show_help()
{
  this->_show_usage();
  std::cout <<  "Options" << std::endl;
  std::cout
     << "  -h [ --help ]             produce help message                    " << std::endl
     << "  -i [ --info ] [arg]       show modules info                       " << std::endl
     << "  -n [ --name ] arg         unique daemon instance name             " << std::endl
     << "  -d [ --daemonize ]        run as daemon                           " << std::endl
     << "  -c [ --coredump ]         allow core dump                         " << std::endl
     << "  -a [ --autoup ] [arg]     auto restart daemon                     " << std::endl
     << "  -C [ --config-path ] arg  path to the configuration file          " << std::endl
     << "  -G [ --generate ] [arg]   generate configuration                  " << std::endl;
  std::cout<< std::endl;

  if ( auto m = _global->modules.lock())
  {
    std::cout << "modules:" << std::endl;
    m->for_each([](const std::string& name, std::weak_ptr<imodule>){
      std::cout << "  " << name <<  std::endl;
    });
  }
}

void core::_show_info()
{
  std::cout << _global->program_name << " version:" << std::endl;
  std::cout << _global->program_version << std::endl;
  std::cout << "----------------------------------------------" << std::endl;
  std::cout << "wfc version:" << std::endl;
  std::cout << _global->wfc_version << std::endl;
}

void core::_show_module_info(const std::string& module_name)
{
  if (auto gm = _global->modules.lock() )
  {
    if (!module_name.empty())
    {
      if ( auto m = gm->get(module_name).lock() )
      {
        std::cout << "----------------------------------------------" << std::endl;
        std::cout << module_name << " module version:" << std::endl;
        std::cout << m->version() << std::endl;
        std::cout << m->description() << std::endl;
      }
    }
    else
    {
      gm->for_each([this](const std::string& name, std::weak_ptr<imodule>)
      {
        this->_show_module_info(name);
      });
    }
  }
}

///
/// generate
///

void core::_generate( const std::string& type, const std::string& path )
{
  if ( auto c = _global->config.lock() )
  {
    c->generate(type, path);
  }
  else
  {
    std::cerr << "Config module not set" << std::endl;
  }
}
*/

}
