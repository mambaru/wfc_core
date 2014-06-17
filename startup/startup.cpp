#include "startup.hpp"
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

/*

namespace {

static void signal_sigint_handler(int)
{
  if ( auto g = global::static_global.lock() )
    if ( auto c = g->core.lock() )
      c->stop();
  exit(0);
}

} // namespace
*/


startup_impl::~startup_impl()
{
  
}

startup_impl::startup_impl(std::weak_ptr<global> g )
  : _global(g)
{
  
}

void startup_impl::reconfigure()
{
}

bool startup_impl::startup( int argc, char* argv[])
{
  return this->_startup(argc, argv);
}

void startup_impl::configure(const startup_config& conf)
{
  CONFIG_LOG_MESSAGE("startup module configured")
  this->_conf = conf;
}


bool startup_impl::_startup(int argc, char** argv)
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
      ::openlog( (p.instance_name+"(wfc_startup)").c_str(), 0, LOG_USER);
      std::stringstream ss;
      ss << "Daemon stop with status: " << status << ". ";
      if ( restart )
        ss << "Restarting...";
      else
        ss << "Do not restarted.";
      
      if ( status!= 0 )
        DAEMON_LOG_FATAL( ss.str() )
      else
        DAEMON_LOG_MESSAGE( ss.str() )
        
      ::syslog(LOG_ERR, ss.str().c_str());
      ::closelog();
    });
  }

  if ( p.coredump )
    ::wfc::dumpable();

  
  /*
  signal(SIGPIPE,  SIG_IGN);
  signal(SIGPOLL,  SIG_IGN);
  signal(SIGINT,   signal_sigint_handler);
  signal(SIGTERM,  signal_sigint_handler);
  */
  
  return true;
}


///
/// help
///

void startup_impl::_show_usage()
{
  std::cout <<  "Usage" << std::endl;
  std::cout <<  "  " << _global->program_name << " -h" << std::endl;
  std::cout <<  "  " << _global->program_name << " [-h] -i [<module name>]" << std::endl;
  std::cout <<  "  " << _global->program_name << " -G [<generate name>] [-C <path>]" << std::endl;
  std::cout <<  "  " << _global->program_name << " [-d] [-c] [-a <timeout>] [-n <instance name>] -C <path>" << std::endl;
}


void startup_impl::_show_help()
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
    m->for_each([](const std::string& name, std::weak_ptr<imodule> /*m*/){
      std::cout << "  " << name <<  std::endl;
    });
  }
}

void startup_impl::_show_info()
{
  std::cout << _global->program_name << " version:" << std::endl;
  std::cout << _global->program_version << std::endl;
  std::cout << "----------------------------------------------" << std::endl;
  std::cout << "wfc version:" << std::endl;
  std::cout << _global->wfc_version << std::endl;
}

void startup_impl::_show_module_info(const std::string& module_name)
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
      gm->for_each([this](const std::string& name, std::weak_ptr<imodule> /*mod*/)
      {
        this->_show_module_info(name);
      });
    }
  }
}

///
/// generate
///

void startup_impl::_generate( const std::string& type, const std::string& path )
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

}
