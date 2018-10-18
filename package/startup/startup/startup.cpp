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

int startup_domain::startup(int argc, char** argv, std::string helpstring)
{
  parse_arguments( _pa, argc, argv);
  if ( auto g = this->global() )
  {
    g->program_name  = _pa.program_name;
    g->instance_name  = _pa.instance_name;
  }

  if ( !_pa.errorstring.empty() )
  {
    std::cerr << "*** ERROR ***" << std::endl;
    std::cerr << _pa.errorstring << std::endl;
    return 1;
  }
  else if ( _pa.usage )
  {
    this->show_usage_();
    return 0;
  }
  else if ( _pa.help )
  {
    if ( _pa.help_options.empty() )
    {
      if ( !helpstring.empty() ) std::cout << helpstring << std::endl << std::endl;
      std::cout << _pa.helpstring << std::endl;
      std::cout << "To generate and format the configuration, use: " << std::endl;
      std::cout << _pa.program_name << " -G | python -mjson.tool " << std::endl;
      std::cout << std::endl;
    }
    else
    {
      for (std::string name : _pa.help_options )
      {
        if ( auto g = this->global() )
        {
          if ( auto p = g->registry.get<icomponent>("component", name, true) )
          {
            std::cout << "*** " << name << " ***" << std::endl;
            std::cout << p->help() << std::endl << std::endl;
          }
          else
          {
            std::cout << "ERROR: component '" << name << "' is not exist." << std::endl << std::endl;
          }
        }
      }
    }
    return 0;
  }
  else if ( _pa.version )
  {
    if ( auto g = this->global() )
    {
      std::cout << _pa.program_name << " " << g->program_build_info->version() 
                << std::endl << g->program_build_info->compiler_version() 
                << std::endl << g->program_build_info->build_date() 
                << std::endl << g->program_build_info->build_type() 
                << std::endl;
    }
    return 0;
  }
  else if ( _pa.info )
  {
    if ( _pa.info_options.empty() )
    {
      this->show_info_("");
      return 0;
    }
    else
    {
      bool flag = true;
      for ( const std::string& i : _pa.info_options )
      {
        flag = flag && this->show_info_(i);
      }
      return flag ? 0 : 2;
    }
  }
  else if ( _pa.module_list )
  {
    if ( auto g = this->global() )
    {
      g->registry.for_each<ipackage>("package", [](const std::string&, std::shared_ptr<ipackage> pkg)
      {
        if ( pkg == nullptr )
          return;
        std::cout << pkg->name() << ":" << std::endl;
        auto modules = pkg->modules();
        for (auto m : modules)
        {
          std::cout << "\t" << m->name() << " " << m->description() << std::endl;
        }
      },
      [](std::shared_ptr<ipackage> l, std::shared_ptr<ipackage> r)->bool
      {
        return l->order() < r->order();
      });
    }
  }
  else if ( _pa.component_list )
  {
    if ( auto g = this->global() )
    {
      g->registry.for_each<ipackage>("package", [](const std::string& , std::shared_ptr<ipackage> pkg)
      {
        if ( pkg == nullptr )
          return;
        std::cout << pkg->name() << ":" << std::endl;
        auto modules = pkg->modules();
        for (auto m : modules)
        {
          auto components = m->components();
          for (auto c : components )
          {
            std::cout << "\t" << c->name() << " [" << c->interface_name() << "] " << c->description() << std::endl;
          }
        }
      },
      [](std::shared_ptr<ipackage> l, std::shared_ptr<ipackage> r)->bool
      {
        return l->order() < r->order();
      });
    }
  }
  else if ( _pa.generate )
  {
    return this->generate_() ? 0 : 3;
  }
  else if ( !_pa.check_config.empty() )
  {
    if ( auto g = this->global() )
    {
      if ( auto c = g->registry.get<iconfig>("config") )
      {
        return c->load_and_configure(_pa.check_config) ? 0 : 4;
      }
    }
  }
  else if ( !_pa.config_path.empty() )
  {
    return this->perform_start_();
  }
  else
  {
    std::cerr << "ERROR: you must specify a configuration file (-C)" << std::endl;
    this->show_usage_();
  }
  return 0;
}


bool startup_domain::ready_for_run()
{
  return _ready;
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

namespace {
  
  struct checked_item
  {
    std::string name;
  };
  
  struct checked_item_json
  {
    JSON_NAME(name)
    typedef wjson::object<
      checked_item,
      wjson::member_list<
        wjson::member<n_name, checked_item, std::string, &checked_item::name>
      >
    > type;
    typedef type::serializer serializer;
    typedef type::target target;
    typedef type::member_list member_list;
  };
  
  bool check_options(const std::string& path, const std::set<std::string>& required )
  {
    std::map<std::string, std::string> conf;
    std::set<std::string> conf_names;
    wjson::dict_map< wjson::raw_value<> >::serializer conf_json;
    std::ifstream f(path);
    wjson::json_error er;
    std::string jsonconf;
    std::copy(
      std::istreambuf_iterator<char>(f),
      std::istreambuf_iterator<char>(),
      std::back_inserter(jsonconf)
    );

    conf_json(conf, std::begin(jsonconf), std::end(jsonconf), &er );
    
    if (er)
    {
      std::cerr << "ERROR: " << wjson::strerror::message(er) << std::endl;
      return false;
    }
    
    bool status = true;
    for (const auto& item : conf )
    {
      if ( wjson::parser::is_object( item.second.begin(), item.second.end() ) )
      {
        if ( !conf_names.insert(item.first).second )
        {
          std::cerr << "ERROR: Item '" << item.first << "' already exist. Check the configuration for duplication of entities." << std::endl;
          status = false;
        }
      }
      else if ( wjson::parser::is_array( item.second.begin(), item.second.end() ) )
      {
        std::vector<checked_item> checked_list;
        wjson::array< std::vector<checked_item_json> >::serializer checked_list_json;
        wjson::json_error er;
        checked_list_json(checked_list, item.second.begin(), item.second.end(), &er);
        if (er)
        {
          std::cerr << wjson::strerror::message(er) << std::endl;
          return false;
        }
        for (const auto& chk: checked_list )
        {
          if ( !conf_names.insert(chk.name).second )
          {
            std::cerr << "ERROR: instance '" << chk.name 
                      << "' already exist. Check the configuration for duplication of entities." << std::endl;
            status = false;
          }
        }
      }
    }//
    
    for ( const auto& name : required )
    {
      if ( conf_names.count(name) == 0 )
      {
        std::cerr << "ERROR: instance '"<< name << "' not found for program args" << std::endl;
        status = false;
      }
    }
    return status;
  }
}

int startup_domain::perform_start_( )
{
  if ( auto g = this->global() )
  {
    if ( auto c = g->registry.get<iconfig>("config") )
    {
      if ( !c->load_and_configure(_pa.config_path) )
      {
        std::cerr << "Configuration FAIL!" << std::endl;
        return 5;
      }

      std::set<std::string> required;
      for (auto& item : _pa.object_options) required.insert(item.first);
      for (auto& item : _pa.startup_options) required.insert(item.first);
      if ( !check_options(_pa.config_path, required) )
        return 5;
    }
    std::clog << "Program name: " << g->program_name << std::endl;
    std::clog << "Instance name: " << g->instance_name << std::endl;
  }
  
  if ( !_pa.user_name.empty() )
  {
    std::string err;
    if ( !::wfc::change_user(_pa.user_name, &err) )
    {
      std::cerr << "FAIL: cannot set new user name '"<< _pa.user_name <<"': " << err << std::endl;
      return 6;
    }
    std::clog << "New user name: " << _pa.user_name << std::endl;
  }
    
  if ( !_pa.working_directory.empty() )
  {
    std::string err;
    if ( !::wfc::change_working_directory(_pa.working_directory, &err) )
    {
      std::cerr << "FAIL: cannot set new working directory '" << _pa.working_directory << "': " << err << std::endl;
      return 7;
    }
    std::clog << "New working directory: " << _pa.user_name << std::endl;
  }

  if ( !loc_file_pid(_pa.pid_dir, _pa.instance_name) )
  {
    return 8;
  }

  if ( _pa.daemonize )
  {
    if ( _pa.autoup )
      std::clog << "autoup process enabled" << std::endl;
    
    std::clog << "daemonize... see log for startup status" << std::endl;
    
    if ( _pa.wait_daemonize )
      std::clog << "wait for finalize daemon startup" << std::endl;
    
    if ( auto fun = ::wfc::daemonize(_pa.wait_daemonize) )
    {
      if ( auto g = this->global() )
      {
        g->after_start.insert( [fun](){ fun(); return false;} );
      }
    }
  } 
  else if ( _pa.autoup || _pa.wait_daemonize)
  {
    if ( _pa.autoup )
      std::clog << "WARNING: autoup argument ignored. Only with -d worked" << std::endl;
    if (_pa.wait_daemonize)
      std::clog << "WARNING: wait_daemonize argument ignored. Only with -d worked" << std::endl;
  }

  if ( _pa.daemonize && _pa.autoup )
  {
    bool success_autoup = _pa.success_autoup;
    ::wfc::autoup( 
      _pa.autoup_timeout,
      success_autoup,
      nullptr,
      [this](int count, int status, time_t work_time)
      {
        _pa.startup_options.clear();
        if ( auto g = this->global() )
        {
          g->after_start.insert([count, status, work_time](){
            SYSTEM_LOG_BEGIN("------------------------------------------------")
            if ( status != 0 )
            {
              SYSTEM_LOG_ERROR("Daemon stop with status: " << status << " after work time " << work_time << "sec. ")
            }
            else
            {
              SYSTEM_LOG_WARNING("Daemon was killed after work time " << work_time << "sec. ")
            }
            SYSTEM_LOG_WARNING("Restart №" << count)
            SYSTEM_LOG_END("------------------------------------------------")
            return false;
          });
        }
      }
    );
  }
  
  if ( auto g = this->global() )
  {
    g->args.insert(_pa.startup_options);
    g->args.insert(_pa.object_options);
  }
    

  if ( _pa.coredump )
    ::wfc::dumpable();

  _ready = true;
  return 0;
}

///
/// help
///

void startup_domain::show_usage_()
{
  std::cout <<  "Usage:" << std::endl;
  std::cout <<  "  " << this->global()->program_name << " --help" << std::endl;
  std::cout <<  "  " << this->global()->program_name << " --version" << std::endl;
  std::cout <<  "  " << this->global()->program_name << " --info [<package name>]" << std::endl;
  std::cout <<  "  " << this->global()->program_name << " -G [<component name>] [-C <path>]" << std::endl;
  std::cout <<  "  " << this->global()->program_name << " --check-config <path>" << std::endl;
  std::cout <<  "  " << this->global()->program_name << " [-d] [-c] [-a <timeout>] [-n <component name>] -C <config path>" << std::endl;
}

bool startup_domain::show_info_(const std::string& name)
{
  auto g = this->global();

  if ( g==nullptr )
    return false;
  
  if ( !name.empty() )
  {
    if ( auto p = g->registry.get<ipackage>("package", name, true) )
    {
      std::cout << "About Package:" << std::endl;
      this->show_build_info_(p->build_info(), false);
      std::cout << "\tDescription: " << p->description() << std::endl;
      std::cout << "\tList of modules:" << std::endl;
      auto modules = p->modules();
      for( auto m: modules ) 
      {
          std::cout << "\t\t" << m->name() << ". " << m->description() << std::endl;
          auto components = m->components();
          for( auto o: components ) 
          {
            std::cout << "\t\t\t" << o->name() << " - " << o->description() << std::endl;
          }
      }
    }
    else
    {
      std::cout << "Information about the package is not available." << std::endl;
      std::cout << "Package '" << name << "' Not Found!" << std::endl;
      std::cout << "Available packages: " << std::endl;
      g->registry.for_each<ipackage>("package", [this](const std::string& name, std::shared_ptr<ipackage>)
      {
        std::cout << "\t" << name << std::endl;
      });
      return false;
    }
  }
  else
  {
    std::cout << "About Program:" << std::endl;
    this->show_build_info_(g->program_build_info, false);
    std::cout << "About WFC:" << std::endl;
    this->show_build_info_(g->wfc_build_info, false);
    std::cout << "Package List:" << std::endl;

    g->registry.for_each<ipackage>("package", 
      [this](const std::string&, std::shared_ptr<ipackage> p)
      {
        this->show_build_info_(p->build_info(), true);
      }, 
      [](std::shared_ptr<ipackage> left, std::shared_ptr<ipackage> right)
      {
        return left->order() < right->order();
      }
    );
  }
  return true;
}

void startup_domain::show_build_info_(std::shared_ptr<ibuild_info> b, bool shortinfo)
{
  if ( b==nullptr )
    return;
  
  if ( shortinfo )
  {
    std::cout << std::setw(20) << std::right << b->name() 
              << " " << std::setw(17) << std::left << b->version() 
              << " " << b->initial_author() << std::endl;
  }
  else
  {
    std::cout << "\tEnabled: " << b->enabled()  << std::endl;
    std::cout << "\tName: "    << b->name()     << std::endl;
    std::cout << "\tVersion: " << b->version() << std::endl;
    std::cout << "\tCompiler: " << b->compiler_version() << std::endl;
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
  }
}

///
/// generate
///

/// @return false - модуль не найден
bool startup_domain::generate_()
{
  if ( auto g = this->global() )
  {
    if ( auto c = g->registry.get<iconfig>("config") )
    {
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
        return true;
      }
      else
      {
        std::cerr << genstr << std::endl;
      }
    }
  }
  return false;
}

}}
