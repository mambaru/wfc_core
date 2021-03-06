#include "config.hpp"
#include "configuration.hpp"
#include "configuration_json.hpp"

#include <wfc/core/wfcglobal.hpp>
#include <wfc/core/iconfig.hpp>
#include <wfc/core/icore.hpp>
#include <wfc/system/system.hpp>
#include <wfc/logger.hpp>
#include <wfc/module/ipackage.hpp>
#include <wfc/module/imodule.hpp>
#include <wfc/module/icomponent.hpp>
#include <wlog/init.hpp>

#include <string>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <syslog.h>


namespace wfc{ namespace core{

namespace
{
  static void signal_sighup_handler(int);
  inline time_t get_modify_time(const std::string& path);
} // namespace


config::~config()
{

}

config::config()
  : _timer_id(0)
  , _config_changed(0)
{
}

void config::restart()
{
  this->global()->common_workflow->release_timer(_timer_id);
  _timer_id = 0;

  if ( this->options().reload_changed_ms != 0 )
  {
    _timer_id = this->global()->common_workflow->create_timer(
      std::chrono::milliseconds(this->options().reload_changed_ms),
      this->wrap( std::bind(&config::timer_handler_, this), [](){return false;} )
    );
  }
}

void config::start()
{
  if ( this->options().reload_sighup )
  {
    signal(SIGHUP, signal_sighup_handler);
  }

  this->_config_changed = get_modify_time(this->_path);
  this->restart();
}

void config::stop()
{
  signal(SIGHUP, nullptr);
  this->global()->common_workflow->release_timer(_timer_id);
}

bool config::reload_and_reconfigure()
{
  SYSTEM_LOG_BEGIN("Reload Configuration And Reconfigure")
  std::string confstr = load_from_file_(_path);
  if ( confstr.empty() )
    return false;
  configuration mainconf;
  if ( !parse_configure_(_path, confstr, mainconf) )
    return false;
  _mainconf = mainconf;
  if ( auto c = this->get_target<icore>("core") )
  {
    c->core_reconfigure();
  }
  else
  {
    SYSTEM_LOG_ERROR("Core module not found")
  }
  SYSTEM_LOG_END("Reload Configuration And Reconfigure")
  return true;
}


bool config::load_and_configure(std::string path)
{
  std::string confstr = load_from_file_(path);
  if ( confstr.empty() )
    return false;

  configuration mainconf;
  if ( !parse_configure_(path, confstr, mainconf) )
    return false;
  _mainconf = mainconf;
  _path = path;
  return true;
}


bool config::load_and_check(std::string path)
{
  std::string confstr = load_from_file_(path);
  if ( confstr.empty() )
    return false;

  configuration mainconf;
  if ( !parse_configure_(path, confstr, mainconf) )
    return false;
  return true;
}

std::string config::get_config(std::string component_name)
{
  auto itr = _mainconf.find(component_name);
  if (itr==_mainconf.end())
  {
    return std::string();
  }
  return itr->second;
}

bool config::generate_config( const generate_options& go, const std::string& path, std::string& result)
{
  auto g = this->global();
  if ( g == nullptr )
  {
    result = "the system is not initialized";
    return false;
  }

  std::string defarg;

  bool empty_opt = go.empty();

  if ( go.count("all")!=0 )
  {

    defarg = go.at("all");
    empty_opt = go.size() == 1;
  }

  if ( empty_opt )
  {

    std::vector< std::pair<std::string, std::string> > vectconf;
    g->registry.for_each<ipackage>("package", [&vectconf, &defarg](const std::string&, std::shared_ptr<ipackage> pkg)
    {
      if ( pkg == nullptr )
        return;
      auto modules = pkg->modules();
      for (auto m : modules)
      {
        auto components = m->components();
        wlog::disable();
        for (const auto& c : components )
        {
          vectconf.push_back( std::make_pair(c->name(), c->generate(defarg) ) );
        }
        wlog::disable();
      }
    },
    [](std::shared_ptr<ipackage> l, std::shared_ptr<ipackage> r)->bool
    {
      return l->show_order() < r->show_order();
    });

    json::dict_vector< json::raw_value<> >::serializer()(vectconf, std::back_inserter(result));
  }
  else
  {
    configuration mainconf;
    for ( const auto& opt: go )
    {
      if ( auto obj = g->registry.get_object<icomponent>("component", opt.first, true) )
      {
        std::string genopt = opt.second;
        if ( genopt.empty() ) genopt = defarg;
        mainconf[opt.first] = obj->generate(genopt);
      }
      else
      {
        std::stringstream ss;
        ss << "WFC generate error! component '"<< opt.first << "' not found." << std::endl;
        ss << "Available components:" << std::endl;
        g->registry.for_each<ipackage>("package", [&ss](const std::string& package_name, std::shared_ptr<ipackage> pkg)
        {
          if ( pkg == nullptr )
            return;
          auto modules = pkg->modules();
          for (auto m : modules)
          {
            auto components = m->components();
            for (auto c : components )
            {
              ss << std::setw(20) << c->name()  << "\t[" << package_name << " " << m->name() << " ]: " << c->description() << std::endl;
            }
          }
        },
        [](std::shared_ptr<ipackage> l, std::shared_ptr<ipackage> r)->bool
        {
          return l->show_order() < r->show_order();
        });

        result=ss.str();
        return false;
      }
    }
    configuration_json::serializer()(mainconf, std::back_inserter(result));
  }

  this->save_to_file_( path, result);
  return true;
}

bool config::parse_configure_(const std::string& source, const std::string& confstr, configuration& mainconf)
{
  auto g = this->global();

  std::string::const_iterator jsonbeg = confstr.begin();
  std::string::const_iterator jsonend = confstr.end();

  json::json_error e;
  jsonbeg = json::parser::parse_space(jsonbeg, jsonend, &e);
  if (!e)
    configuration_json::serializer()(mainconf, jsonbeg, jsonend, &e);

  if ( e )
  {
    SYSTEM_LOG_ERROR( "Invalid json configuration from '" << source << "': "
        << std::endl << json::strerror::message_trace(e, jsonbeg, jsonend )  )
    return false;
  }

  for ( auto& mconf : mainconf)
  {
    if ( auto m = g->registry.get_object<icomponent>("component", mconf.first, true) )
    {
      if ( !m->parse( mconf.second, &e) )
      {
        SYSTEM_LOG_ERROR(
          "Invalid json configuration from '" << source << "' for '"<< mconf.first << "':" << std::endl
          << json::strerror::message(e) << std::endl
          << json::strerror::trace(e, mconf.second.begin(), mconf.second.end() ) << std::endl
          << "^^^^ Configuration is not valid!"
        )
        return false;
      }
    }
    else
    {
      SYSTEM_LOG_ERROR( "Invalid json configuration from '" << source << "'" )
      SYSTEM_LOG_ERROR( "Module '" << mconf.first << "' not found")

      return false;
    }
  }
  return true;
}

bool config::timer_handler_()
{
  if ( this->_config_changed!=0 )
  {
    time_t t = get_modify_time(this->_path);
    if ( t!=this->_config_changed )
      this->reload_and_reconfigure();
    this->_config_changed = t;
  }
  return true;
}

std::string config::load_from_file_(const std::string& path)
try
{
  std::string confstr;
  std::ifstream fconf(path);
  if ( !fconf.good() )
  {
    SYSTEM_LOG_ERROR( "Bad json configuration file '" << path << "'" );
    return std::string();
  }
  std::copy(
    std::istreambuf_iterator<char>(fconf),
    std::istreambuf_iterator<char>(),
    std::back_inserter(confstr)
  );
  return confstr;
}
catch(const std::exception& e)
{
  SYSTEM_LOG_ERROR( "Invalid json configuration file '" << path << "': " << e.what() );
  return std::string();
}

void config::save_to_file_(const std::string& path, const std::string& confstr)
{
  if ( !path.empty() )
  {
    std::ofstream fconf(path);
    std::copy(
      confstr.begin(),
      confstr.end(),
      std::ostreambuf_iterator<char>(fconf)
    );
  }
}

namespace
{
  static void signal_sighup_handler(int)
  {
    SYSTEM_LOG_WARNING("SIGHUP signal handler")
    if ( auto g = wfcglobal::static_global )
    {
      boost::asio::post(g->io_context, [g]()
      {
        if ( auto c = g->registry.get_target<iconfig>("config") )
          c->reload_and_reconfigure();
      });
    }
  }

  inline time_t get_modify_time(const std::string& path)
  {
    struct ::stat st;
    if ( ::stat( path.c_str(), &st) != -1)
      return st.st_mtime;
    return static_cast<time_t>(-1);
  }

} // namespace


}}
