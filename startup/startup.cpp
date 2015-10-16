//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2013-2015
//
// Copyright: See COPYING file that comes with this distribution
//

#include "startup.hpp"
#include "domain/parse_arguments.hpp"

#include <wfc/core/iconfig.hpp>
#include <wfc/module/imodule.hpp>
#include <wfc/module/ipackage.hpp>
#include <wfc/system/system.hpp>
#include <wfc/logger.hpp>
#include <wfc/asio.hpp>

#include <fstream>
#include <syslog.h>

#include <sys/file.h>
#include <errno.h>


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
    g->instance_name  = _pa.instance_name;
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
    return this->perform_start_();
  }
  else
  {
  }
  return false;
}

namespace {

  bool loc_file_pid( std::string path, std::string name)
  {
    if ( !path.empty() && path.back()!='/' )
      path += '/';

    std::string fname = path + name + ".pid";
    int pid_file = ::open(fname.c_str(), O_CREAT | O_RDWR, 0666);
    if ( pid_file == -1)
    {
      std::cout << fname << " open error: " << strerror(errno) << std::endl;
      return false;
    }
    int rc = ::flock(pid_file, LOCK_EX | LOCK_NB);
    if (rc) 
    {
      if( EWOULDBLOCK == errno)
      {
        std::cout << fname << " blocked another instance" << std::endl;
        std::cout << "FAIL :  another instance '"<< name <<"' is running" << std::endl;
        return false; // another instance is running
      }
      std::cout << fname << " blocked error: " << strerror(errno) << std::endl;
    }
    else 
    {
      std::cout << fname << " blocked this instance" << std::endl;
    }
    return true;
  }
}

bool startup_domain::perform_start_( )
{
  if ( auto g = this->global() )
  {
    std::cout << "Program name: " << g->program_name << std::endl;
    std::cout << "Instance name: " << g->instance_name << std::endl;
    if ( !loc_file_pid(_pa.pid_dir, _pa.instance_name) )
    {
      return false;
    }

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
    ::wfc::autoup(_pa.autoup_timeout, [wthis](bool restart, int status, time_t work_time)->bool
    {
      if ( auto pthis = wthis.lock() )
      {
        std::stringstream ss;
        ss << "Daemon stop with status: " << status << " after work time " << work_time << "sec. " ;
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
        return restart;
      }
      return false;
    });
  }

  if ( _pa.coredump )
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
      std::cout << "About Package:" << std::endl;
      this->show_build_info_(p->build_info(), false);
      auto modules = p->modules();
      for( auto m: modules ) 
      {
          std::cout << "\t\t" << m->name() << ". " << m->description() << std::endl;
          auto components = m->components();
          for( auto o: components ) 
          {
              std::cout << "\t\t\t" << o->name() << " [" << o->interface_name() 
                        << "]. " << o->description() << std::endl;
          }
      }
    }
  }
  else
  {
    std::cout << "About Program:" << std::endl;
    this->show_build_info_(g->program_build_info, false);
    std::cout << "About WFC:" << std::endl;
    this->show_build_info_(g->wfc_build_info, false);
    std::cout << "Package List:" << std::endl;
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
    std::cout << '\t' << b->name() << "(" << b->version() << ", " << b->commit_date() << ") " << b->project_author() << std::endl;
  }
  else
  {
    std::cout << "\tName: "    << b->name() << std::endl;
    std::cout << "\tEnabled: " << b->enabled() << std::endl;
    std::cout << "\tVersion: " << b->version() << std::endl;
    std::cout << "\tBuild Type: " << b->build_type() << std::endl;
    std::cout << "\tBuild Date: " << b->build_date() << std::endl;
    std::cout << "\tBranch: "  << b->branch() << std::endl;
    std::cout << "\tCommit: "  << b->commit() << std::endl;
    std::cout << "\tCommit Date: " << b->commit_date() << std::endl;
    std::cout << "\tCommit Author: " << b->commit_author() << std::endl;
    std::cout << "\tCommit Message: " << b->commit_message() << std::endl;
    std::cout << "\tProject Author: " << b->project_author() << std::endl;
    std::cout << "\tAll Authors: " << b->all_authors() << std::endl;
  }
}

/*
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
*/
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
