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

  std::cout << "sunrise" << std::endl;

  this->_sunrise();
  
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

  _global->program_name = p.program_name;
  
  if (p.usage)
    _show_usage();
  
  if (p.help)
    _show_help();

  if (p.info)
  {
    if ( p.info_name.empty() )
      _show_info();
    _show_module_info(p.info_name);
  }

  if (p.generate)
    _generate(p.generate_name, p.config_path);

  if ( p.usage || p.help || p.info || p.generate )
    return false;
    

  if ( p.daemonize )
    daemonize();

  if ( auto c = _global->config.lock() )
  {
    if ( !c->parse_config(p.config_path) )
      return false;
  }


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

  if ( auto c = _global->config.lock() )
    c->configure(p.config_path);

  return true;
}

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
     << "  -i [ --info ] arg         produce help message                    " << std::endl
     << "  -n [ --name ] arg         unique daemon instance name             " << std::endl
     << "  -d [ --daemonize ]                                                " << std::endl
     << "  -c [ --coredump ]                                                 " << std::endl
     << "  -a [ --autoup ] arg                                               " << std::endl
     << "  -C [ --config-path ] arg  path to the configuration file          " << std::endl
     << "  -G [ --generate ] arg     generate of configuration type          " << std::endl;
  std::cout<< std::endl;
  
  if ( auto m = _global->modules.lock())
  {
    std::cout << "modules:" << std::endl;
    m->for_each([](const std::string& name, std::weak_ptr<imodule> m){
      std::cout << "  " << name <<  std::endl;
    });
  }
}

void core::_show_info()
{
  std::cout << _global->program_name << " version:" << std::endl;
  std::cout << _global->program_version << std::endl;
  std::cout << "----------------------------------------------" << std::endl;
  std::cout << "comet version:" << std::endl;
  std::cout << _global->comet_version << std::endl;
}


void core::_show_module_info(const std::string& module_name)
{
  if (auto gm = _global->modules.lock() )
  {
    if (!module_name.empty())
    {
      if ( auto m = gm->find(module_name).lock() )
      {
        std::cout << "----------------------------------------------" << std::endl;
        std::cout << module_name << " module version:" << std::endl;
        std::cout << m->version() << std::endl;
        std::cout << m->description() << std::endl;
      }
    }
    else
    {
      gm->for_each([this](const std::string& name, std::weak_ptr<imodule> mod)
      {
        this->_show_module_info(name);
      });
    }
  }
}

void core::_generate( const std::string& type, const std::string& path )
{
  if ( auto c = _global->config.lock() )
  {
    std::cout <<  c->generate(type, path) << std::endl;
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
  if (auto gm = _global->modules.lock() )
  {
    gm->for_each([&modules](const std::string& name, std::weak_ptr<imodule> m){
      modules.push_back( module_pair( name, m.lock()) );
    });
  }

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
