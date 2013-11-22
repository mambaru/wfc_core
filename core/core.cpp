#include "core.hpp"
#include "detail/po.hpp"
#include <comet/inet/epoller.hpp>
#include <comet/core/global.hpp>
#include <comet/core/imodule.hpp>
#include <comet/core/iconfig.hpp>
#include <comet/system/system.hpp>
#include <vector>
#include <string>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <syslog.h>

namespace mamba{ namespace comet{

core::~core()
{
  
}

void core::reconfigure()
{
  
}

void core::run( int argc, char* argv[], std::weak_ptr<global> gl )
{
  _global = gl.lock();
  _mux = std::make_shared<inet::epoller>();
  _global->mux = _mux;

  if ( !this->_startup(argc, argv) )
      return;
}

void core::stop( )
{
  
}

void core::configure(const core_config& conf)
{
  _conf = conf;
}

bool core::_startup(int argc, char** argv)
{
  detail::po p = detail::po::parse(argc, argv);

  if (p.help)
  {
    if ( p.module_help.empty() )
      _show_help();
    _show_module_help(p.module_help);
  }

  if (p.generate)
    _generate(p.generate_name);

  if ( p.help || p.generate )
    return false;

  if ( p.daemonize )
    daemonize();

  if ( p.daemonize && p.autoup )
  {
    autoup(p.autoup_timeout, [p](bool restart, int status)
    {
      openlog( (p.instance_name+"(comet_core)").c_str(), 0, LOG_USER);
      std::stringstream ss;
      ss << "Daemon stop with status: " << status << ". ";
      if ( restart )
        ss << "Restarting...";
      else
        ss << "Do not restarted.";
      syslog(LOG_ERR, ss.str().c_str());
      closelog();
    });
  }

  if ( p.coredump )
    dumpable();

   return true;
}

void core::_show_help()
{
  
  std::cout
     << "  -h [ --help ] arg         produce help message                    " << std::endl
     << "  -n [ --name ] arg         unique daemon instance name             " << std::endl
     << "  -d [ --daemonize ]                                                " << std::endl
     << "  -c [ --coredump ]                                                 " << std::endl
     << "  -a [ --autoup ] arg                                               " << std::endl
     << "  -C [ --config-path ] arg  path to the configuration file          " << std::endl
     << "  -G [ --generate ] arg     generate of configuration type          " << std::endl;
}

void core::_show_module_help(const std::string& module_name)
{
  if (!module_name.empty())
  {
    if ( auto m = _global->modules.find(module_name).lock() )
    {
      std::cout << "----------------------------------------------" << std::endl;
      std::cout << module_name << ":" << std::endl;
      std::cout << m->version() << std::endl;
      std::cout << m->description() << std::endl;
    }
  }
  else
  {
    _global->modules.for_each([this](const std::string& name, std::weak_ptr<imodule> mod)
    {
      this->_show_module_help(name);
    });
  }
}

void core::_generate( const std::string& generate_name )
{
  if ( auto c = _global->config.lock() )
  {
    std::cout <<  c->generate(generate_name) << std::endl;
    // For JSON format: cat aaa.conf | python -mjson.tool
  }
  else
  {
    std::cout << "Config module not set" << std::endl;
  }
}

void core::_sunrise()
{
  module_vector modules;
  _global->modules.for_each([&modules](const std::string& name, std::weak_ptr<imodule> m){
    modules.push_back( module_pair( name, m.lock()) );
  });

  std::sort(modules.begin(), modules.end(), [](const module_pair& left, const module_pair& right)->bool {
    return left.second->startup_priority() < right.second->startup_priority();
  } );

  _configure(modules);
  _initialize(modules);
  _start(modules);
  
}

void core::_configure(const module_vector& modules)
{
  std::for_each(modules.begin(), modules.end(),
    [config=_global->config.lock()](const module_pair& m)
  {
    if ( config )
      m.second->configure(config->get_config(m.first));
  });
}

void core::_initialize(const module_vector& modules)
{
  std::for_each(modules.begin(), modules.end(), [](const module_pair& m)
  {
    m.second->initialize();
  });
}

void core::_start(const module_vector& modules)
{
  std::for_each(modules.begin(), modules.end(), [](const module_pair& m)
  {
    m.second->start();
  });
}

void core::_stop(const module_vector& modules)
{
  std::for_each(modules.begin(), modules.end(), [](const module_pair& m)
  {
    m.second->stop();
  });
  
}


}}
