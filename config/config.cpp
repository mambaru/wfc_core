#include "config.hpp"

#include <comet/inet/epoller.hpp>
#include <comet/core/global.hpp>
#include <comet/core/imodule.hpp>
#include <comet/core/iconfig.hpp>
#include <comet/system/system.hpp>
#include <vector>
#include <string>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <syslog.h>

namespace mamba{ namespace comet{

config::~config()
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
  return false;
}

void config::configure(const std::string& path)
{
  
}

std::string config::get_config(const std::string& name)
{
  return std::string("{}");
}

std::string config::generate(const std::string& type)
{
  return std::string();
}


}}
