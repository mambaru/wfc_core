//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2013-2015
//
// Copyright: See COPYING file that comes with this distribution
//

#include "startup.hpp"
#include "domain/parse_arguments.hpp"
#include "detail/po.hpp"

#include <wfc/core/iconfig.hpp>
#include <wfc/module/imodule.hpp>
#include <wfc/module/ipackage.hpp>
#include <wfc/system/system.hpp>
#include <wfc/logger.hpp>

#include <fstream>
#include <syslog.h>


namespace wfc{

startup_domain::~startup_domain()
{
}

bool startup_domain::startup( int argc, char* argv[])
{
  return this->startup_(argc, argv);
}


bool startup_domain::startup_(int argc, char** argv)
{
  program_arguments pa;
  parse_arguments( pa, argc, argv);
  std::cout << "startup_" << pa.help << std::endl;
  if ( pa.help )
  {
    std::cout << pa.helpstring << std::endl;
  }
  else if ( pa.info )
  {
    std::cout << "TODO: info" << std::endl;
  }
  else if ( pa.generate )
  {
    std::cout << "TODO: bbbb" << std::endl;
    if ( !this->generate_(pa) )
    {
      std::cerr << "the system is not initialized";
    }
  }
  return false;
  /*
  detail::po2 p2 = detail::po2::parse(argc, argv);
  std::cout << "----po----" << std::endl;
  std::cout << "P2 ready" << std::endl;
  std::cout << "----po----" << std::endl;
  */
  detail::po p = detail::po::parse(argc, argv);

  
  this->global()->program_name = p.program_name;
  this->global()->instance_name = p.instance_name.empty()
                                 ? p.instance_name
                                 : p.program_name;
  
  this->global()->options = p.module_options;
  
  if (p.usage)
    this->show_usage_();
  
  if (p.help)
    this->show_help_();

  if (p.info)
  {
    if ( p.info_name.empty() )
      this->show_info_();
    this->show_module_info_(p.info_name);
  }

  if (p.generate)
  {
    this->generate_(p.generate_name, p.config_path);
  }

  if ( p.usage || p.help || p.info || p.generate )
  {
    return false;
  }

  if ( auto c = this->global()->registry.get<iconfig>("config") )
    c->load_and_parse(p.config_path);

  if ( p.daemonize )
    ::wfc::daemonize();

  if ( p.daemonize && p.autoup )
  {
    ::wfc::autoup(p.autoup_timeout, [p](bool restart, int status, time_t /*worktime*/)->bool
    {
      ::openlog( (p.instance_name+"(wfc_startup)").c_str(), 0, LOG_USER);
      std::stringstream ss;
      ss << "Daemon stop with status: " << status << ". ";
      if ( restart )
        ss << "Restarting...";
      else
        ss << "Do not restarted.";

      if ( status!= 0 )
      {
        DOMAIN_LOG_FATAL( ss.str() )
      }
      else
      {
        DOMAIN_LOG_MESSAGE( ss.str() )
      }
        
      ::syslog(LOG_ERR, ss.str().c_str());
      ::closelog();
      return restart;
    });
  }

  if ( p.coredump )
    ::wfc::dumpable();
 
  return true;
}


///
/// help
///

void startup_domain::show_usage_()
{
  std::cout <<  "Usage" << std::endl;
  std::cout <<  "  " << this->global()->program_name << " -h" << std::endl;
  std::cout <<  "  " << this->global()->program_name << " [-h] -i [<module name>]" << std::endl;
  std::cout <<  "  " << this->global()->program_name << " -G [<generate name>] [-C <path>]" << std::endl;
  std::cout <<  "  " << this->global()->program_name << " [-d] [-c] [-a <timeout>] [-n <instance name>] -C <path>" << std::endl;
}


void startup_domain::show_help_()
{
  this->show_usage_();
  std::cout <<  "Options" << std::endl;
  std::cout
     << "  -h [ --help ]             produce help message                    " << std::endl
     << "  -i [ --info ] [arg]       show modules info                       " << std::endl
     << "  -n [ --name ] arg         unique daemon instance name             " << std::endl
     << "  -d [ --daemonize ]        run as daemon                           " << std::endl
     << "  -c [ --coredump ]         allow core dump                         " << std::endl
     << "  -a [ --autoup ] [arg]     auto restart daemon                     " << std::endl
     << "  -C [ --config-path ] arg  path to the configuration file          " << std::endl
     << "  -G [ --generate ] [arg]   generate configuration                  " << std::endl
     << "  --NAME-KEY=[VALUE]        module defined options                  " << std::endl;
  std::cout<< std::endl;

  /*if ( auto m = _global->modules)*/
  //{
  std::cout << "modules:" << std::endl;
  this->global()->registry.for_each<imodule>("module", []( const std::string& name, std::shared_ptr<imodule> /*m*/){
    std::cout << "  " << name <<  std::endl;
  });
  //}
}

void startup_domain::show_info_()
{
  std::cout << this->global()->program_name << " version:" << std::endl;
  //std::cout << this->global()->program_version << std::endl;
  std::cout << "----------------------------------------------" << std::endl;
  std::cout << "wfc version:" << std::endl;
  //std::cout << this->global()->wfc_version << std::endl;
}

void startup_domain::show_module_info_(const std::string& module_name)
{
  if (auto g = this->global() )
  {
    if (!module_name.empty())
    {
      if ( auto m = g->registry.get<ipackage>("package", module_name) )
      {
        std::cout << "----------------------------------------------" << std::endl;
        std::cout << module_name << " module version:" << std::endl;
        std::cout << m->build_info() << std::endl;
        std::cout << m->description() << std::endl;
      }
      else
      {
        std::cout << "ERROR: Package '" << module_name << "' not found" << std::endl;
      }
    }
    else
    {
      g->registry.for_each<imodule>("package", [this](const std::string& name, std::shared_ptr<imodule> )
      {
        this->show_module_info_(name);
      });
    }
  }
}

///
/// generate
///

/// @return false - модуль не найден
bool startup_domain::generate_(const program_arguments& pa)
{
  auto g = this->global();
  if ( g==nullptr )
    return false;

  auto c = g->registry.get<iconfig>("config");
  if ( c==nullptr )
    return false;

  std::string genstr;
  if ( c->generate_config(pa.generate_options, genstr) )
  {
    if ( pa.config_path.empty() )
    {
      std::cout << genstr << std::endl;
    }
    else
    {
      std::ofstream fconf(pa.config_path);
      std::copy(
        genstr.begin(),
        genstr.end(),
        std::ostreambuf_iterator<char>(fconf)
      );
      std::cout << "TODO: help for formating" << std::endl;
    }
  }
  else
  {
    std::cerr << genstr << std::endl;
  }
  return true;
}

void startup_domain::generate_( const std::string& /*type*/, const std::string& /*path*/ )
{
  //TODO: Удалить
  //if ( auto c = _global->config )
  /*
  if ( auto c = this->global()->registry.get<iconfig>("config") )
  {
    c->generate_and_write(type, path);
  }
  else
  {
    std::cerr << "Config module not set" << std::endl;
  }
  */
}

}
