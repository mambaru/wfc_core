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
  if ( auto g = global::static_global.lock() )
    if ( auto c = g->config.lock() )
      c->reconfigure();
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
  /*_timer_owner = std::make_unique<callback_owner>();
  if ( !_global )
    return;
  */
  
  /*
  _global->idle( std::chrono::milliseconds(1000), _timer_owner->callback([this]()
    {
      if ( !this->_conf.enabled )
        return callback_status::ready;

      if ( this->_conf.reload_changed && this->_config_changed!=0 )
      {
        time_t t = get_modify_time(this->_path);
        if ( t!=this->_config_changed )
          this->reconfigure();
        this->_config_changed = t;
      }
      return callback_status::ready;
    }));
    */

}

config::config(std::shared_ptr<global> gl)
  : _config_changed(0)
  , _global(gl)
{
  
  
  if ( _global )
  {
    std::cout << "_global->idle.push_back" << std::endl;
    _global->idle.push_back( callback<callback_status>([this]()
    //_global->idle( std::chrono::milliseconds(1000), callback([this]()
    {
      std::cout << "idle config" << std::endl;
      if ( !this->_conf.enabled )
        return callback_status::ready;

      if ( this->_conf.reload_changed && this->_config_changed!=0 )
      {
        time_t t = get_modify_time(this->_path);
        if ( t!=this->_config_changed )
          this->reconfigure();
        this->_config_changed = t;
      }
      return callback_status::ready;
    }));
  }
}


/*
void config::_check_and_reload_config_file(time_t now)
{
  if ( _observe_timeout==0 || _config_path.empty() )
    return ;

  if ( now < _observe_time)
    return;

  _observe_time = now + _observe_timeout;

  time_t modify_time = get_modify_time(_config_path);

  if ( modify_time > _modify_time )
  {
    _modify_time = modify_time;
    this->reload_config();
  }
}
*/

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
  if ( auto c = _global->core.lock() )
    c->reconfigure();
  _init_timer();
}

/*
bool config::parse_config(const std::string& path)
{
  // TDOD:
  return true;
}*/

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
  
  /*
  try
  {
    beg = aj::parser::parse_space(beg, end);
    */
  /*
  _path = path;
  std::string confstr = _load_from_file(path);
  */
  
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
    // std::cout << "[[[" << e.message( confstr.begin(), confstr.end() ) << "]]]" << std::endl;
    throw config_error(ss.str());
  }

  if ( auto modules = _global->modules.lock() )
  {
    for ( auto& mconf : mainconf)
    {
      if ( auto m = modules->get(mconf.first).lock() )
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
  }
}

std::string config::get_config(std::string name)
{
  auto itr = _mainconf.find(name);
  if (itr==_mainconf.end())
  {
    /*
    std::cout << name << " not found" << std::endl;
    std::cout << _mainconf.size() << std::endl;
    */
    return std::string();
  }
  return itr->second;
}

std::string config::generate(std::string type, std::string path)
{
  std::string confstr;
  configuration mainconf;

  if (auto gm = _global->modules.lock())
  {
    gm->for_each([&mainconf, &type](const std::string& name, std::weak_ptr<imodule> module){
      if (auto m = module.lock() )
        mainconf[name] = m->generate(type);
    } );
  }
  
  /*
    mainconf["a"]="1234";
    mainconf["b"]="234";
  */
  
  configuration_json::serializer()(mainconf, std::back_inserter(confstr));

  // std::cout << "GENERATE" << std::endl;
  // std::cout << confstr << std::endl;

  if ( path.empty() )
    path = "generate.conf";
  
   _save_to_file(path, confstr);

  std::cout << "generated '"<< type << "' type to " << path << std::endl;
  std::cout << "For JSON format: cat "<< path << " | python -mjson.tool" << std::endl;

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
