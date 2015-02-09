
#include "core.hpp"
#include "core_module.hpp"
#include "core_config_json.hpp"
#include "core_build_info.h"

#include <wfc/core/global.hpp>


namespace wfc{

imodule::priority core_module::startup_priority()  const
{
  return imodule::priority::core;  
}

imodule::priority core_module::shutdown_priority() const
{
  return imodule::priority::core;
}

std::string core_module::version() const
{
  return core_build_info_string;
}

std::string core_module::description() const
{
  return std::string("Core module");
}

std::string core_module::generate(const std::string& /*type*/)  const
{
  std::string result;  
  core_config conf;
  //conf.enabled = true;
  core_config_json::serializer()(conf, std::back_inserter(result));
  return result;
}

bool core_module::parse_config(const std::string& confstr)
{
  core_config conf;
  core_config_json::serializer()(conf, confstr.begin(), confstr.end());
  return true;
}

void core_module::create( const std::string& /*name*/, std::shared_ptr<global> g )
{
  _core = std::make_shared<core>();
  if ( g != nullptr )
  {
    g->registry.set("core", _core);
  }
}

void core_module::configure(const std::string& confstr, const std::string&)
{
  core_config_json::serializer()(_config, confstr.begin(), confstr.end());
  _core->configure(_config);
}

void core_module::initialize(const std::string&)
{
  
}

void core_module::start(const std::string&)
{
  
}

void core_module::stop(const std::string&)
{
  
}

void core_module::shutdown(const std::string&)
{
  
}

void core_module::idle()
{
  
}

}
