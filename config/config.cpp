#include "config.hpp"

#include <comet/inet/epoller.hpp>
#include <comet/core/global.hpp>
#include <comet/core/imodule.hpp>
#include <comet/core/iconfig.hpp>
#include <comet/core/except.hpp>
#include <comet/system/system.hpp>
#include <comet/logger.hpp>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <syslog.h>

#include "configuration.hpp"
#include "configuration_json.hpp"
namespace mamba{ namespace comet{

namespace {

static void signal_sighup_handler(int)
{
  if ( auto g = global::static_global.lock() )
    if ( auto c = g->config.lock() )
      c->reconfigure();
}

} // namespace


config::~config()
{
  
}

config::config(std::shared_ptr<global> gl)
  : _global(gl)
{
  
}

void config::configure(const config_config& conf)
{
  signal(SIGHUP, signal_sighup_handler);
}

// iconfig
void config::reconfigure()
{
  CONFIG_LOG_MESSAGE("config::reconfigure()")
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
      if ( auto m = modules->find(mconf.first).lock() )
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


    /*
    std::for_each(_mainconf.begin(), _mainconf.end() []( auto& p){
      
    });
    */
    /*
    modules->for_each([](const std::string& name, std::weak_ptr<imodule> module)
    {
      auto itr = _mainconf.find(name);
      if ( itr == _mainconf.end() )
        throw std::domain_error("invalid module name");
      
      try
      {
      }
      catch(const json::json_error& e)
      {
      }
    });
    
  }
  
}*/

/*
configuration config::_configure()
{
  configuration mainconf;
  std::string confstr = _load_from_file(path);
  configuration_json::serializer()(mainconf, confstr.begin(), confstr.end());
  return mainconf;
}
*/

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

}}
