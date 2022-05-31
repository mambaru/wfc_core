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

#include <iostream>
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
  /* Базовый domain_object доступен ограниченно, система не инициализирована работаем через global */
  parse_arguments( _pa, argc, argv);

  auto g = this->global();
  if ( g == nullptr )
  {
    std::cerr << "*** ERROR ***" << std::endl;
    std::cerr << "global is nullptr" << std::endl;
    std::cerr << "wfcglobal::static_global initialization required" << std::endl;
  }

  g->program_name  = _pa.program_name;
  g->instance_name  = _pa.instance_name;

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
      for (std::string helpparams : _pa.help_options )
      {
        std::string components_names;
        std::string component_args;

        size_t pos_params = helpparams.find(':');
        if ( pos_params == std::string::npos )
        {
          components_names=helpparams;
        }
        else
        {
          components_names = helpparams.substr(0, pos_params);
          component_args = helpparams.substr(pos_params + 1);
        }

        std::vector<std::string> component_list;
        boost::split( component_list, components_names, boost::is_any_of(",") );

        for (const std::string& component_name: component_list)
        {
          if ( auto p = g->registry.get_object<icomponent>("component", component_name, true) )
          {
            if ( component_list.size() > 1 )
              std::cout << "***** " << "Help for '" << component_name << "' *****" << std::endl;
            std::cout << p->help(component_args) << std::endl << std::endl;
          }
          else if ( component_name!="all" )
          {
            std::cout << "ERROR: component '" << component_name  << "' is not exist." << std::endl << std::endl;
          }
          else
          {
            g->registry.for_each<icomponent>("component", [component_args](const std::string& cmp_name, std::shared_ptr<icomponent> p1 )
            {
              std::cout << "***** " << "Help for '" << cmp_name << "' *****" << std::endl;
              std::cout << p1->help(component_args) << std::endl << std::endl;
            });
          }
        }
      }
    }
    return 0;
  }
  else if ( _pa.version )
  {
    std::cout << _pa.program_name << " " << g->program_build_info->version()
              << std::endl << g->program_build_info->compiler_version()
              << std::endl << g->program_build_info->build_date()
              << std::endl << g->program_build_info->build_type()
              << std::endl;
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
      return l->show_order() < r->show_order();
    });
  }
  else if ( _pa.component_list )
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
      return l->show_order() < r->show_order();
    });
  }
  else if ( _pa.generate )
  {
    return this->generate_() ? 0 : 3;
  }
  else if ( !_pa.check_config.empty() )
  {
    if ( auto c = g->registry.get_target<iconfig>("config") )
    {
      return c->load_and_configure(_pa.check_config) ? 0 : 4;
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

void startup_domain::stop()
{
}

namespace {

  int loc_file_pid(const std::string& fname)
  {
    int pid_file = ::open(fname.c_str(), O_CREAT | O_RDWR, 0666);
    if ( pid_file == -1)
    {
      std::cerr << fname << " open ERROR: " << strerror(errno) << std::endl;
      return pid_file;
    }

    int rc = ::flock(pid_file, LOCK_EX | LOCK_NB);

    if (rc)
    {
      std::cerr << fname << " blocked ERROR: " << strerror(errno) << std::endl;
      if( EWOULDBLOCK == errno)
      {
        std::cerr << fname << " blocked for another instance" << std::endl;
        return -2; // another instance is running
      }
      return -1;
    }
    else
    {
      std::clog << fname << " blocked for this instance. " << std::endl;
    }
    return pid_file;
  }

  int write_loc_pid( int fileid, pid_t pid)
  {
    if ( -1 ==  ftruncate(fileid, 0) )
    {
      std::cerr << "ERROR ftruncate pid: " << strerror(errno) << std::endl;
      return -1;
    }

    char buffer[128]={0};
    wjson::value<pid_t>::serializer()( pid, std::begin(buffer) );
    if ( -1 == ::write(fileid, buffer, strlen(buffer) ) )
    {
      std::cerr << "ERROR write pid: " << strerror(errno) << std::endl;
      return -1;
    }
    return 0;
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
  auto g = this->global();

  if ( auto c = g->registry.get_target<iconfig>("config") )
  {
    g->ini.clear();
    std::copy(_pa.ini_list.begin(), _pa.ini_list.end(), std::back_inserter(g->ini) );
    std::copy(_pa.startup_ini_list.begin(), _pa.startup_ini_list.end(), std::back_inserter(g->ini) );

    if ( !c->load_and_configure(_pa.config_path) )
    {
      std::cerr << "Configuration FAIL!" << std::endl;
      return 5;
    }
  }

  std::set<std::string> required;
  for (auto& item : _pa.object_options) required.insert(item.first);
  for (auto& item : _pa.startup_options) required.insert(item.first);
  if ( !check_options(_pa.config_path, required) )
    return 5;

  std::clog << "Program name: " << g->program_name << std::endl;
  std::clog << "Instance name: " << g->instance_name << std::endl;

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

  _pid_path = _pa.pid_dir;
  if ( !_pid_path.empty() && _pid_path.back()!='/' )
      _pid_path += '/';

  _pid_path += _pa.instance_name + ".pid";
  int pid_file = loc_file_pid(_pid_path);

  if ( pid_file < 0 )
  {
    if ( pid_file == -2 )
      std::cerr << "FAIL :  another instance '"<< _pa.instance_name <<"' is running" << std::endl;
    _pid_path.clear();
    return 8;
  }

  g->after_start.insert(std::bind<bool>(&startup_domain::init_shutdown_timer_, this) );

  if ( _pa.daemonize )
  {
    if ( _pa.autoup )
      std::clog << "autoup process enabled" << std::endl;

    std::clog << "daemonize... see log for startup status" << std::endl;

    if ( _pa.wait_daemonize )
      std::clog << "wait for finalize daemon startup" << std::endl;

    if ( auto fun = ::wfc::daemonize(_pa.wait_daemonize) )
    {
      g->after_start.insert( [fun](){ fun(); return false;} );
    }
  }
  else if ( _pa.wait_daemonize )
  {
    std::clog << "WARNING: wait_daemonize argument ignored. Only with -d worked" << std::endl;
  }

  bool working_proccess = true;
  if ( _pa.autoup )
  {
    bool success_autoup = _pa.success_autoup;
    working_proccess = !wfc::autoup(
      _pa.autoup_timeout,
      success_autoup,
      [this](pid_t pid1, int count, int status, time_t work_time) -> bool
      {
        int pid_file1 = loc_file_pid(this->_pid_path);
        write_loc_pid(pid_file1, pid1);

        std::clog << "Process PID=" << pid1 << " terminated with status " << status << " and will be restarted" << std::endl;
        std::clog << "Work Time: " << work_time << " seconds" << std::endl;
        std::clog << "Restart count: " << count << std::endl;
        return true;
      },
      [this, g](int count, int status, time_t work_time)
      {
        this->_pa.startup_options.clear();
        this->_pa.startup_ini_list.clear();


        g->after_start.insert([count, status, work_time, this]()
        {
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
          if ( !_pa.shutdown_time.empty() )
          {
            SYSTEM_LOG_WARNING("Reset shutdown_time. Previous value: " << _pa.shutdown_time)
            _pa.shutdown_time.clear();
          }
          SYSTEM_LOG_END("------------------------------------------------")
          return false;
        });
      }
    );
  }

  // Это основной и не мониторящий процесс (получаем pid после демонизации)
  if ( working_proccess )
  {
    pid_t pid = ::getpid();
    if ( -1 == write_loc_pid(pid_file, pid) )
    {
      return 9;
    }

    g->after_start.insert( [pid](){
      SYSTEM_LOG_MESSAGE( "Process identifier (PID): " << pid );
      return false;
    } );
    
    std::string pid_path = _pid_path;
    g->after_stop.insert([pid_path](){
      int code = ::remove(pid_path.c_str());
      if ( code != 0 )
      {
        SYSTEM_LOG_ERROR("ERROR: pid file: " << strerror(errno) );
      }
      return false;
    });

  }
  else if ( _pa.autoup )
    return 0;

  g->args.clear();
  g->args.insert(_pa.startup_options);
  g->args.insert(_pa.object_options);

  bool enable_dump = _pa.coredump;
  g->after_start.insert( [enable_dump](){
    int resdump = wfc::dumpable(enable_dump);
    if ( resdump == -2 )
    {
      SYSTEM_LOG_WARNING( "DUMPABLE (--coredump) is not set because prctl not detected" );
    }
    else if (resdump != 0)
    {
      SYSTEM_LOG_WARNING( "DUMPABLE (--coredump) is not set because prctl error: " <<  strerror(errno) );
    }
    else
    {
      SYSTEM_LOG_MESSAGE("SET_DUMPABLE:" << enable_dump)
    }
    return false;
  });

  _ready = true;
  return 0;
}


bool startup_domain::init_shutdown_timer_()
{
  if ( !_pa.shutdown_time.empty() )
  {
    this->get_common_workflow()->safe_post(
      _pa.shutdown_time,
      std::bind(&startup_domain::init_working_timer_, this)
    );
  }
  else if ( !_pa.working_time.empty() )
  {
    this->init_working_timer_();
  }

  return false;
}

void startup_domain::init_working_timer_()
{
  if ( _pa.restart_by_timer )
  {
    SYSTEM_LOG_MESSAGE("*** Restart by timer! ***")
    this->get_common_workflow()->post(
      _pa.working_time,
      std::bind(wfc_restart)
    );
  }
  else
  {
    SYSTEM_LOG_MESSAGE("*** Shutdown by timer! ***")
    this->get_common_workflow()->post(
      _pa.working_time,
      std::bind(wfc_exit)
    );
  }

}


///
/// help
///

void startup_domain::show_usage_()
{
  auto g = this->global();

  std::cout <<  "Usage:" << std::endl;
  std::cout <<  "  " << g->program_name << " --help" << std::endl;
  std::cout <<  "  " << g->program_name << " --version" << std::endl;
  std::cout <<  "  " << g->program_name << " --info [<package name>]" << std::endl;
  std::cout <<  "  " << g->program_name << " -G [<component name>] [-C <path>]" << std::endl;
  std::cout <<  "  " << g->program_name << " --check-config <path>" << std::endl;
  std::cout <<  "  " << g->program_name << " [-d] [-c] [-a <timeout>] [-n <component name>] -C <config path>" << std::endl;
}

bool startup_domain::show_info_(const std::string& package_name)
{
  auto g = this->global();

  if ( g==nullptr )
    return false;

  if ( !package_name.empty() )
  {
    if ( auto p = g->registry.get_object<ipackage>("package", package_name, true) )
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
      std::cout << "Package '" << package_name << "' Not Found!" << std::endl;
      std::cout << "Available packages: " << std::endl;
      g->registry.for_each<ipackage>("package", [](const std::string& pkgname, std::shared_ptr<ipackage>)
      {
        std::cout << "\t" << pkgname << std::endl;
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
        return left->show_order() < right->show_order();
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
  auto g = this->global();

  if ( auto c = g->registry.get_target<iconfig>("config") )
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
  return false;
}

}}
