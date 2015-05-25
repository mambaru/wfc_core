#pragma once

#include <string>
#include <vector>
#include <map>

namespace wfc{

struct program_arguments
{
  bool help = false;
  bool usage = false;
  bool generate = false;
  bool info = false;

  bool daemonize = false;
  bool coredump = false;
  bool autoup = false;
  time_t autoup_timeout = 0; // Убрать в конфиг 

  std::string errorstring;
  std::string helpstring;

  std::string program_name;
  std::string instance_name;
  std::string config_path;
  std::string configex_path; // для модуля wfc-config
  std::string ini_path;

  typedef std::map<std::string, std::string> map1;
  typedef std::map<std::string, map1> map2;

  std::vector<std::string> info_packages;
  map1 generate_options;
  map2 instance_options;
};

}