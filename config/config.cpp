#include "config.hpp"

//#include <wfc/inet/epoller.hpp>
#include <wfc/core/global.hpp>
#include <wfc/module/imodule.hpp>
#include <wfc/core/iconfig.hpp>
#include <wfc/core/except.hpp>
#include <wfc/system/system.hpp>
#include <wfc/logger.hpp>
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
  if ( auto g = global::static_global )
  {
    g->io_service.post([g]()
    {
      if ( auto c = g->config )
        c->reconfigure();
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

void config::_init_timer()
{

}

config::config(std::shared_ptr<global> gl)
  : _config_changed(0)
  , _global(gl)
{
  if ( _global )
  {
    _global->idle.push_back( this->callback<void>([this]()
    {
      if ( !this->_conf.enabled )
        return;

      if ( this->_conf.reload_changed && this->_config_changed!=0 )
      {
        time_t t = get_modify_time(this->_path);
        if ( t!=this->_config_changed )
          this->reconfigure();
        this->_config_changed = t;
      }
    }));
  }
}

void config::configure(const config_config& conf)
{
  _conf = conf;
  signal(SIGHUP, signal_sighup_handler);
  if ( _conf.reload_changed )
    _config_changed = get_modify_time(_path);
  _init_timer();
}

void config::reconfigure()
{
  CONFIG_LOG_MESSAGE("config::reconfigure()")
  std::string confstr = _load_from_file(_path);
  configuration mainconf;
  try
  {
    _parse_configure(_path, confstr, mainconf);
  }
  catch(const config_error& e)
  {
    CONFIG_LOG_ERROR(e.what())
    CONFIG_LOG_ERROR("Configuration ignored!")
    return;
  }
  _mainconf = mainconf;
  if ( auto c = _global->core )
    c->reconfigure();
  _init_timer();
}


void config::initialize(std::string path)
{
  std::string confstr = _load_from_file(path);
  configuration mainconf;
  try
  {
    _parse_configure(path, confstr, mainconf);
  }
  catch(const config_error& e)
  {
    std::cerr << e.what() << std::endl;
    throw;
  }

  _mainconf = mainconf;
  _path = path;
}

void config::_parse_configure(std::string source, std::string confstr, configuration& mainconf)
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
    throw config_error(ss.str());
  }

  /*if ( auto modules = _global->modules )
  {*/
    for ( auto& mconf : mainconf)
    {
      std::cout << "ещлЖ "<< mconf.first << std::endl;
      if ( auto m = _global->registry.get<imodule>(mconf.first) )
      {
        jsonbeg = mconf.second.begin();
        jsonend = mconf.second.end();
        try
        {
          jsonbeg = json::parser::parse_space(jsonbeg, jsonend);
          if ( !m->parse_config( std::string(jsonbeg, jsonend)) )
          {
            std::stringstream ss;
            ss << "Invalid json configuration from '" << source << "' for module '"<< mconf.first << "':" << std::endl;
            ss << "Configuration is not valid! see documentation for module";
            throw config_error(ss.str());  
          }
        }
        catch(const json::json_error& e)
        {
          std::stringstream ss;
          ss << "Invalid json configuration from '" << source << "' for module '"<< mconf.first << "':" << std::endl;
          ss << e.message( jsonbeg, jsonend );
          throw config_error(ss.str());
        }
        catch(const std::exception& e)
        {
          std::stringstream ss;
          ss << "Invalid json configuration from '" << source << "' for module '"<< mconf.first << "':" << std::endl;
          ss << e.what();
          throw config_error(ss.str());
        }
      }
      else
      {
        std::stringstream ss;
        ss << "Invalid json configuration from '" << source << "':" << std::endl;
        ss << "Module '" << mconf.first << "' not found"; 
        throw config_error(ss.str());
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

std::string config::generate(std::string type, std::string path)
{
  std::string confstr;
  configuration mainconf;

  /*if (auto gm = _global->modules)
  {*/
    _global->registry.for_each<imodule>([&mainconf, &type](const std::string& name, std::shared_ptr<imodule> module){
      if (module!=nullptr)
        mainconf[name] = module->generate(type);
    } );
  /*}*/
  
  
  configuration_json::serializer()(mainconf, std::back_inserter(confstr));


  if ( path.empty() )
    path = "generate.conf";
  
   _save_to_file(path, confstr);

  std::clog << "generated '"<< type << "' type to " << path << std::endl;
  std::clog << "For JSON format: cat "<< path << " | python -mjson.tool" << std::endl;

  return confstr;
}

std::string config::_load_from_file(const std::string& path)
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

void config::_save_to_file(const std::string& path, const std::string& strconf)
{
  std::ofstream fconf(path);
  std::copy(
    strconf.begin(),
    strconf.end(),
    std::ostreambuf_iterator<char>(fconf)
  );
}

}
