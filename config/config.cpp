#include "config.hpp"

#include <comet/inet/epoller.hpp>
#include <comet/core/global.hpp>
#include <comet/core/imodule.hpp>
#include <comet/core/iconfig.hpp>
#include <comet/system/system.hpp>
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

config::~config()
{
  
}

config::config(std::shared_ptr<global> gl)
  : _global(gl)
{
  
}


void config::configure(const config_config& conf)
{
}


// iconfig
void config::reconfigure()
{
  
}

bool config::parse_config(const std::string& path)
{
  // TDOD:
  return true;
}

void config::configure(const std::string& path)
{
  std::string confstr = _load_from_file(path);
  std::cout << "config::configure " << confstr << std::endl;
  configuration_json::serializer()(_mainconf, confstr.begin(), confstr.end());
  std::cout << _mainconf.size() << std::endl;
}

std::string config::get_config(const std::string& name)
{
  auto itr = _mainconf.find(name);
  if (itr==_mainconf.end())
  {
    std::cout << name << " not found" << std::endl;
    std::cout << _mainconf.size() << std::endl;
    return std::string("{}");
  }
  return itr->second;
}

std::string config::generate(const std::string& type, const std::string& path)
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
  std::cout << "GENERATE" << std::endl;
  std::cout << confstr << std::endl;

  if (path.empty())
    _save_to_file("generate.conf", confstr);
  else
    _save_to_file(path, confstr);
  
  return std::string();
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
