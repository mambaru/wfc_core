//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2013-2015
//
// Copyright: See COPYING file that comes with this distribution
//

#include "startup.hpp"
#include "parse_arguments.hpp"

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


namespace wfc{ namespace core{

startup_domain::~startup_domain()
{
}

bool startup_domain::startup(int argc, char** argv, std::string helpstring)
{
  parse_arguments( _pa, argc, argv);
  if ( auto g = this->global() )
  {
    g->program_name  = _pa.program_name;
    g->instance_name  = _pa.instance_name;
    g->args.insert(_pa.instance_options);
  }

  if ( !_pa.errorstring.empty() )
  {
    std::cerr << "*** ERROR ***" << std::endl;
    std::cerr << _pa.errorstring << std::endl;
  }
  else if ( _pa.usage )
  {
    this->show_usage_();
  }
  else if ( _pa.help )
  {
    if ( !helpstring.empty() ) std::cout << helpstring << std::endl << std::endl;
    std::cout << _pa.helpstring << std::endl;
    std::cout << "Для генерации и форматировани конфигурации используете: " << std::endl;
    std::cout << _pa.program_name << " -G | python -mjson.tool " << std::endl;
    std::cout << std::endl;
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
    std::cerr << "ERROR: для запуска необходимо указать файл конфигурации" << std::endl;
    this->show_usage_();
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
      std::cerr << fname << " open error: " << strerror(errno) << std::endl;
      return false;
    }
    int rc = ::flock(pid_file, LOCK_EX | LOCK_NB);
    if (rc) 
    {
      if( EWOULDBLOCK == errno)
      {
        std::cerr << fname << " blocked for another instance" << std::endl;
        std::cerr << "FAIL :  another instance '"<< name <<"' is running" << std::endl;
        return false; // another instance is running
      }
      std::cerr << fname << " blocked error: " << strerror(errno) << std::endl;
    }
    else 
    {
      std::clog << fname << " blocked for this instance" << std::endl;
    }
    return true;
  }
}

bool startup_domain::perform_start_( )
{
  if ( auto g = this->global() )
  {
    std::clog << "Program name: " << g->program_name << std::endl;
    std::clog << "Instance name: " << g->instance_name << std::endl;
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
  {
    if ( _pa.autoup )
    {
      std::clog << "autoup process enabled" << std::endl;
    }
    std::clog << "daemonize... see log for startup status" << std::endl;
    ::wfc::daemonize();
  } 
  else if ( _pa.autoup )
  {
    std::clog << "WARNING: autoup argument ignored. Only with -d worked" << std::endl;
  }

  if ( _pa.daemonize && _pa.autoup )
  {
    bool success_autoup = _pa.success_autoup;
    std::weak_ptr<startup_domain> wthis = this->shared_from_this();
    ::wfc::autoup( _pa.autoup_timeout, [wthis, success_autoup](bool restart, int status, time_t work_time)->bool
    {
      if ( auto pthis = wthis.lock() )
      {
        std::stringstream ss;
        ss << "Daemon stop with status: " << status << " after work time " << work_time << "sec. " ;
        if ( !restart && status == 0 && success_autoup)
          restart = true;
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
  std::cout <<  "Usage:" << std::endl;
  std::cout <<  "  " << this->global()->program_name << " --help" << std::endl;
  std::cout <<  "  " << this->global()->program_name << " --info [<module name>]" << std::endl;
  std::cout <<  "  " << this->global()->program_name << " -G [<component name>] [-C <path>]" << std::endl;
  std::cout <<  "  " << this->global()->program_name << " [-d] [-c] [-a <timeout>] [-n <instance name>] -C <path>" << std::endl;
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
}

void startup_domain::show_build_info_(std::shared_ptr<ibuild_info> b, bool shortinfo)
{
  if ( b==nullptr )
    return;
  
  if ( shortinfo )
  {
    std::cout << '\t' << b->name() << "\t" << b->version() << " " << b->initial_author() << std::endl;
  }
  else
  {
    std::cout << "\tEnabled: " << b->enabled()  << std::endl;
    std::cout << "\tName: "    << b->name()     << std::endl;
    std::cout << "\tVersion: " << b->version();
    if ( !b->verex().empty() )
      std::cout << "[" << b->verex() << "]";
    std::cout << "-" << b->build_count() << std::endl;

    std::cout << "\tBuild Type: "     << b->build_type()     << std::endl;
    std::cout << "\tBuild Date: "     << b->build_date()     << std::endl;
    std::cout << "\tBuild Flags: "    << b->build_flags()    << std::endl;
    std::cout << "\tBranch: "         << b->branch()         << std::endl;
    std::cout << "\tCommit: "         << b->commit()         << std::endl;
    std::cout << "\tCommit Date: "    << b->commit_date()    << std::endl;
    std::cout << "\tCommit Author: "  << b->commit_author()  << std::endl;
    std::cout << "\tCommit Message: " << b->commit_message() << std::endl;
    std::cout << "\tAll Authors: "    << b->all_authors()    << std::endl;
    std::cout << "\tInitial Author: " << b->initial_author() << std::endl;
    std::cout << std::endl;
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

}}
