#include "config.hpp"
#include "configuration.hpp"
#include "configuration_json.hpp"

#include <wfc/core/wfcglobal.hpp>
#include <wfc/core/iconfig.hpp>
#include <wfc/core/icore.hpp>
#include <wfc/core/vars.hpp>
#include <wfc/system/system.hpp>
#include <wfc/logger.hpp>
#include <wfc/module/ipackage.hpp>
#include <wfc/module/imodule.hpp>
#include <wfc/module/icomponent.hpp>
#include <wlog/init.hpp>
#include <boost/filesystem.hpp>

#include <string>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <syslog.h>
#include <iostream>


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

  if ( auto g = this->global() )
  {
    std::string confpath = _path;
    auto confmap = _changed_map;


    std::shared_ptr<std::string> show_config;
    if ( this->options().show_config )
    {
      show_config = std::make_shared<std::string>( this->load_config(_path) );
    }


    g->after_start.insert( [confpath, confmap, show_config](){
      SYSTEM_LOG_MESSAGE( "Main config file: " << confpath );
      for ( auto item : confmap )
      {
        SYSTEM_LOG_MESSAGE( "Additional config file: " << item.first );
      }

      if ( show_config != nullptr )
      {
        SYSTEM_LOG_MESSAGE("Startup config: " << std::endl << *show_config )
      }

      return false;
    });
  }

  _changed_map[_path] = get_modify_time(this->_path);

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
  std::string confstr = load_from_file_(_path, true);
  if ( confstr.empty() )
    return false;
  configuration mainconf;
  if ( !parse_configure_(_path, confstr, &mainconf) )
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

std::string config::load_config(std::string path)
{
  bool despace = false;
  if ( auto g = this->global() )
    despace = g->despace;

  std::string config_json;
  std::string confstr = load_from_file_(path, false);
  if ( !despace )
    return confstr;

  configuration mainconf;
  if ( !parse_configure_(path, confstr, &mainconf) )
  { return confstr; }


  configuration_json::serializer()(mainconf, std::back_inserter(config_json));

  confstr.clear();
  wjson::parser::despace( config_json.begin(), config_json.end(), std::back_inserter(confstr), nullptr);
  return confstr;
}

bool config::load_and_configure(std::string path)
{
  if ( auto g = this->global() )
  {
    path = g->find_config(path);
  }

  std::string confstr = load_from_file_(path, false);
  if ( confstr.empty() )
  { return false; }

  configuration mainconf;
  if ( !parse_configure_(path, confstr, &mainconf) )
  { return false; }

  _mainconf = mainconf;
  _path = path;
  return true;
}


bool config::load_and_check(std::string path)
{
  std::string confstr = load_from_file_(path, false);
  if ( confstr.empty() )
  { return false; }

  configuration mainconf;
  if ( !parse_configure_(path, confstr, &mainconf) )
  {  return false; }
  return true;
}

std::string config::get_config(std::string component_name)
{
  auto itr = conf_find( component_name, _mainconf );
  if (itr==_mainconf.end() )
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
        mainconf.push_back({opt.first,obj->generate(genopt)});
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

namespace{
  /*
   * конфигурация это std::vector< std::pair<std::string, std::string> >
   * что позволяет обявлять разные компоненты одного модулея в разных местах
   * а не в одном массиве:
   * т.е. переформатирует
   * {"module":[{comp1}], "module":[{comp2}] }
   * в
   * {"module":[{comp1},{comp2}] }
   * Если указат despace то в сыром json конфигурации компонента удаляет
   * комментарии и пробелы
   */
  template<typename Map>
  bool normalize_json_multimap(const std::string& str, bool despace, Map* map, std::string* err)
  {
    typedef configuration multi_config_t;
    typedef configuration_json multi_config_json;

    std::string::const_iterator jsonbeg = str.begin();
    std::string::const_iterator jsonend = str.end();
    json::json_error e;
    jsonbeg = json::parser::parse_space(jsonbeg, jsonend, &e);

    multi_config_t multi_config;
    if (!e)
    {
      multi_config_json::serializer()(multi_config, jsonbeg, jsonend, &e);
    }

    if (e)
    {
      if (err!=nullptr)
        *err = wjson::strerror::message_trace(e, jsonbeg, jsonend );
      return false;
    }

    for ( auto item : multi_config )
    {
      if ( despace )
      {
        std::string despace_item;
        wjson::parser::despace(item.second.begin(), item.second.end(), std::back_inserter(despace_item), &e);
        if (e)
        {
          if (err!=nullptr)
            *err = wjson::strerror::message_trace(e, jsonbeg, jsonend );
          return false;
        }
        item.second = despace_item;
      }
      auto itr = conf_find(item.first, *map);
      if ( itr == map->end() )
      {
        map->push_back(item);
      }
      else
      {
        jsonbeg = item.second.begin();
        jsonend = item.second.end();
        jsonbeg = wjson::parser::parse_space(jsonbeg, jsonend, &e);

        if (!e)
        {
          typedef wjson::vector_of< wjson::raw_value<std::string> > raw_vector_json;
          std::vector< std::string> first, second;
          raw_vector_json::serializer()(first, item.second.begin(), item.second.end(), &e);
          if (!e)
          {
            raw_vector_json::serializer()(second, itr->second.begin(), itr->second.end(), &e);
            if (!e)
            {
              std::copy( first.begin(), first.end(), std::back_inserter(second));
              itr->second.clear();
              raw_vector_json::serializer()(second, std::back_inserter(itr->second) );
            }
          }
        }

        if (e )
        {
          if (err!=nullptr)
            *err = wjson::strerror::message_trace(e, jsonbeg, jsonend );
          return false;
        }

      }
    }

    return !e;
  }
}

bool config::parse_configure_(const std::string& source, const std::string& confstr, configuration* mainconf)
{
  auto g = this->global();

  std::string strerr;
  if ( !normalize_json_multimap(confstr, g->despace, mainconf, &strerr) )
  {
    SYSTEM_LOG_ERROR( "Invalid json configuration from '" << source << "': "
        << std::endl << strerr  )
    return false;
  }

  for ( auto& mconf : *mainconf)
  {
    if ( auto m = g->registry.get_object<icomponent>("component", mconf.first, true) )
    {
      json::json_error e;
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
  for (auto& file: _changed_map)
  {
    if ( file.second!=0 )
    {
      time_t t = get_modify_time(file.first);
      if ( t!=file.second )
      {
        file.second = t;
        this->reload_and_reconfigure();
        return true;
      }
    }
  }
  return true;
}

std::string config::load_from_file_(const std::string& path, bool is_reload)
try
{
  std::vector<std::string> args_ini;
  std::vector<std::string> opt_ini;
  std::vector<std::string> orig_opt_ini = this->options().ini;

  boost::filesystem::path pp(path);
  if ( auto g = this->global() )
  {
    for (const std::string& file : g->ini)
    {
      boost::filesystem::path p(file);
      if ( !pp.parent_path().empty() && p.parent_path().empty())
      {
        args_ini.push_back( (pp.parent_path()/p.filename()).string() );
      }
    }
  }

  if ( !is_reload )
  {
    wfc::vars v(std::bind( &wfcglobal::find_config, this->global(), std::placeholders::_1 ));
    v.add_ini(args_ini);
    if (v.status() )
      v.parse_file(path);

    if ( !v.status() )
    {
      SYSTEM_LOG_ERROR("Bad configuration (initial pre-parse)'" << path << "': "
                    << static_cast<int>(v.error_code())
                    << ": "<< v.error_message()  )
      return std::string();
    }
    configuration mainconf;
    bool status = this->parse_configure_(path, v.result(), &mainconf);
    if (!status)
      return std::string();

    std::string err;
    if ( status && conf_count("config", mainconf) > 0 )
    {
      configuration config_conf;
      if ( auto g = this->global() )
      {
        if ( status &= normalize_json_multimap(conf_find("config", mainconf)->second, g->despace, &config_conf, &err) )
        {
          if ( conf_count("ini", config_conf) > 0 )
          {
            std::string ini_json = conf_find("ini", config_conf)->second;
            wjson::json_error je;
            wjson::vector_of_strings<>::serializer()(orig_opt_ini, ini_json.begin(), ini_json.end(), &je);
            status &= !je;
            if (je)
              err = wjson::strerror::message_trace(je, ini_json.begin(), ini_json.end());
          }
        }
      }
    }

    if ( !status )
    {
      SYSTEM_LOG_ERROR("Bad configuration (initial pre-config)'" << path << "': " << err)
      return std::string();
    }
  }

  for (const std::string& file : orig_opt_ini)
  {
    boost::filesystem::path p(file);
    if ( !pp.parent_path().empty() && p.parent_path().empty())
    {
      opt_ini.push_back( (pp.parent_path()/p.filename()).string() );
    }
  }

  wfc::vars v(std::bind( &wfcglobal::find_config, this->global(), std::placeholders::_1 ));

  v.add_ini(opt_ini);
  v.add_ini(args_ini);

  if ( v.parse_file(path) )
  {
    for ( const auto& file: v.include_map() )
    {
      _changed_map[file.first] = get_modify_time(file.first);
    }
    return v.result();
  }

  SYSTEM_LOG_ERROR( "Bad configuration (parse)'" << path << "': " << v.error_message() );
  return std::string();
}
catch(const std::exception& e)
{
  SYSTEM_LOG_ERROR( "Bad configuration (exception) '" << path << "': " << e.what() );
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
