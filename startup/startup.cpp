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
  parse_arguments( _pa, argc, argv);
  if ( auto g = this->global() )
  {
    g->program_name  = _pa.program_name;
    g->instance_name = _pa.instance_name.empty()
                                 ? _pa.program_name
                                 : _pa.instance_name;
  }
  
  if ( _pa.usage )
  {
    this->show_usage_();
  }
  else if ( _pa.help )
  {
    std::cout << _pa.helpstring << std::endl;
  }
  else if ( _pa.info )
  {
    if ( _pa.info_options.empty() )
    {
      this->show_info_("");
    }
    else
    {
      for ( const std::string& i : _pa.info_options )
      {
        this->show_info_(i);
      }
    }
  }
  else if ( _pa.generate )
  {
    if ( !this->generate_() )
    {
      std::cerr << "the system is not initialized";
    }
  }
  else if ( !_pa.config_path.empty() )
  {
    this->perform_start_();
    return true;
  }
  else
  {
  }
  return false;
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
      /*this->show_info_()*/{};
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
    ::wfc::autoup(p.autoup_timeout, [p](bool restart, int status, time_t)->bool
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

void startup_domain::perform_start_( )
{
  if ( auto g = this->global() )
  {
    if ( auto c = g->registry.get<iconfig>("config") )
    {
      c->load_and_parse(_pa.config_path);
    }
  }

  if ( _pa.daemonize )
    ::wfc::daemonize();

  if ( _pa.daemonize && _pa.autoup )
  {
    std::weak_ptr<startup_domain> wthis = this->shared_from_this();
    ::wfc::autoup(_pa.autoup_timeout, [wthis](bool restart, int status, time_t)->bool
    {
      if ( auto pthis = wthis.lock() )
      {
        //::openlog( (_pa.instance_name+"(wfc_startup)").c_str(), 0, LOG_USER);
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
        
        SYSLOG_LOG_MESSAGE( ss.str() )
        // ::syslog(LOG_ERR, ss.str().c_str());
        // ::closelog();
        return restart;
      }
      return false;
    });
  }

  if ( _pa.coredump )
    ::wfc::dumpable();

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

  std::cout << "modules:" << std::endl;
  this->global()->registry.for_each<imodule>("module", []( const std::string& name, std::shared_ptr<imodule> /*m*/){
    std::cout << "  " << name <<  std::endl;
  });
}

void startup_domain::show_info_(const std::string& name)
{
  auto g = this->global();

  if ( g==nullptr )
    return;
  
  if ( !name.empty() )
  {
    if ( auto p = g->registry.get<ipackage>("package", name) )
    {
      this->show_build_info_(p->build_info(), false);
      auto modules = p->modules();
      for( auto m: modules ) 
      {
          std::cout << "\t\t" << m->name() << ". " << m->description() << std::endl;
          auto objects = m->objects();
          for( auto o: objects ) 
          {
              std::cout << "\t\t\t" << o->name() << "[" << o->interface_name() << "]. " << m->description() << std::endl;
          }
      }
    }
  }
  else
  {
    this->show_build_info_(g->program_build_info, false);
    this->show_build_info_(g->wfc_build_info, false);
    g->registry.for_each<ipackage>("package", [this](const std::string& /*name*/, std::shared_ptr<ipackage> p)
    {
      this->show_build_info_(p->build_info(), true);
    });
  }
  
  /*
  std::cout << this->global()->program_name << " version:" << std::endl;
  std::cout << "----------------------------------------------" << std::endl;
  std::cout << "wfc version:" << std::endl;
  */
}

void startup_domain::show_build_info_(std::shared_ptr<ibuild_info> b, bool shortinfo)
{
  if ( b==nullptr )
    return;
  
  if ( shortinfo )
  {
    std::cout << '\t' << b->name() << "(" << b->version() << ", " << b->date() << ") " << b->project() << std::endl;
  }
  else
  {
    std::cout << "\tName: " << b->name() << std::endl;
    std::cout << "\tEnabled: " << b->enabled() << std::endl;
    std::cout << "\tVersion: " << b->version() << std::endl;
    std::cout << "\tBranch: " << b->branch() << std::endl;
    std::cout << "\tCommit: " << b->commit() << std::endl;
    std::cout << "\tCommit author: " << b->author() << std::endl;
    std::cout << "\tCommit message: " << b->message() << std::endl;
    std::cout << "\tDate: " << b->date() << std::endl;
    std::cout << "\tProject author: " << b->project() << std::endl;
    std::cout << "\tAuthors: " << b->authors() << std::endl;
  }
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
bool startup_domain::generate_()
{
  auto g = this->global();
  if ( g==nullptr )
    return false;

  auto c = g->registry.get<iconfig>("config");
  if ( c==nullptr )
    return false;

  std::string genstr;
  if ( c->generate_config(_pa.generate_options, _pa.config_path, genstr) )
  {
    if ( _pa.config_path.empty() )
    {
      std::cout << genstr << std::endl;
    }
    else
    {
      std::clog << "generated write to file: " << _pa.config_path << std::endl;
      std::clog << "For JSON format: cat "<< _pa.config_path << " | python -mjson.tool" << std::endl;
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
