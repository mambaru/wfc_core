#include "config.hpp"

//#include <wfc/inet/epoller.hpp>
#include <wfc/core/global.hpp>
#include <wfc/core/iconfig.hpp>
#include <wfc/core/icore.hpp>
#include <wfc/system/system.hpp>
#include <wfc/logger.hpp>
#include <wfc/module/icomponent.hpp>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <syslog.h>

#include "configuration.hpp"
#include "configuration_json.hpp"
namespace wfc{

namespace {

static void signal_sighup_handler(int)
{
  if ( auto g = wfcglobal::static_global )
  {
    g->io_service.post([g]()
    {
      if ( auto c = g->registry.get<iconfig>("config") )
        c->reload_and_reconfigure();
    });
  }
}

static time_t get_modify_time(const std::string& path)
{
  struct stat st;
  if ( stat( path.c_str(), &st) != -1)
    return st.st_mtime;
  return static_cast<time_t>(-1);
}

} // namespace


config::~config()
{
  
}

config::config()
  : _config_changed(0)
{
}

void config::start(const std::string& /*arg*/)
{
  if ( this->options().reload_sighup )
  {
    signal(SIGHUP, signal_sighup_handler);
  }
  
  this->_config_changed = get_modify_time(this->_path);
  if ( auto g = this->global() )
  {
    g->idle.push_back( [this]()->bool
    {
      if ( this->options().reload_changed && this->_config_changed!=0 )
      {
        time_t t = get_modify_time(this->_path);
        if ( t!=this->_config_changed )
          this->reload_and_reconfigure();
        this->_config_changed = t;
      }
      return true;
    });
  }
}

void config::reload_and_reconfigure()
{
  CONFIG_LOG_BEGIN("Reload Configuration And Reconfigure")
  std::string confstr = load_from_file_(_path);
  configuration mainconf;
  try
  {
    parse_configure_(_path, confstr, mainconf);
  }
  catch(const std::domain_error& e)
  {
    CONFIG_LOG_ERROR(e.what())
    CONFIG_LOG_ERROR("Configuration ignored!")
    return;
  }
  _mainconf = mainconf;
  if ( auto c = this->global()->registry.get<icore>("core") )
  {
    CONFIG_LOG_DEBUG("core_reconfigure")
    c->core_reconfigure();
  }
  else
  {
    CONFIG_LOG_ERROR("Core module not found")
  }
  CONFIG_LOG_END("Reload Configuration And Reconfigure")
}


void config::load_and_parse(std::string path)
{
  std::string confstr = load_from_file_(path);
  configuration mainconf;
  try
  {
    parse_configure_(path, confstr, mainconf);
  }
  catch(const std::domain_error& e)
  {
    std::cerr << e.what() << std::endl;
    throw e;
  }

  _mainconf = mainconf;
  _path = path;
}

void config::parse_configure_(std::string source, std::string confstr, configuration& mainconf)
{
  std::string::const_iterator jsonbeg = confstr.begin();
  std::string::const_iterator jsonend = confstr.end();
  
  try
  {
    jsonbeg = json::parser::parse_space(jsonbeg, jsonend);
    configuration_json::serializer()(mainconf, jsonbeg, jsonend);
  }
  catch(const json::json_error& e)
  {
    std::stringstream ss;
    ss << "Invalid json configuration from '" << source << "':" << std::endl;
    ss << e.message( jsonbeg, jsonend );
    throw std::domain_error(ss.str());
  }

  /*if ( auto modules = _global->modules )
  {*/
    for ( auto& mconf : mainconf)
    {
      if ( auto m = this->global()->registry.get<icomponent>("component", mconf.first) )
      {
        jsonbeg = mconf.second.begin();
        jsonend = mconf.second.end();
        try
        {
          jsonbeg = json::parser::parse_space(jsonbeg, jsonend);
          if ( !m->parse( std::string(jsonbeg, jsonend)) )
          {
            std::stringstream ss;
            ss << "Invalid json configuration from '" << source << "' for module '"<< mconf.first << "':" << std::endl;
            ss << "Configuration is not valid! see documentation for module";
            throw std::domain_error(ss.str());  
          }
        }
        catch(const json::json_error& e)
        {
          std::stringstream ss;
          ss << "Invalid json configuration from '" << source << "' for module '"<< mconf.first << "':" << std::endl;
          ss << e.message( jsonbeg, jsonend );
          throw std::domain_error(ss.str());
        }
        catch(const std::exception& e)
        {
          std::stringstream ss;
          ss << "Invalid json configuration from '" << source << "' for module '"<< mconf.first << "':" << std::endl;
          ss << e.what();
          throw std::domain_error(ss.str());
        }
      }
      else
      {
        std::stringstream ss;
        ss << "Invalid json configuration from '" << source << "':" << std::endl;
        ss << "Module '" << mconf.first << "' not found"; 
        throw std::domain_error(ss.str());
      }
    }
  //}
}

std::string config::get_config(std::string name)
{
  auto itr = _mainconf.find(name);
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

  configuration mainconf;
  if ( go.empty() )
  {
    g->registry.for_each<icomponent>("component", [&mainconf](const std::string& name, std::shared_ptr<icomponent> obj)
    {
      if (obj!=nullptr)
      {
        mainconf[name] = obj->generate("");
      }
    });
  }
  else
  {
    for ( const auto& opt: go )
    {
      if ( auto obj = g->registry.get<icomponent>("component", opt.first) )
      {
        mainconf[opt.first] = obj->generate(opt.second);
      }
      else
      {
        std::stringstream ss;
        ss << "WFC generate error! object '"<< opt.first << "' not found";
        result=ss.str();
        return false;
      }
    }
  }
  configuration_json::serializer()(mainconf, std::back_inserter(result));
  this->save_to_file_( path, result);
  return true;
}

std::string config::load_from_file_(const std::string& path)
{
  std::string confstr;
  std::ifstream fconf(path);
  std::copy(
    std::istreambuf_iterator<char>(fconf),
    std::istreambuf_iterator<char>(),
    std::back_inserter(confstr)
  );
  return confstr;
}

void config::save_to_file_(const std::string& path, const std::string& strconf)
{
  if ( !path.empty() )
  {
    std::ofstream fconf(path);
    std::copy(
      strconf.begin(),
      strconf.end(),
      std::ostreambuf_iterator<char>(fconf)
    );
  }
}

}
