#pragma once

#include <string>
#include <vector>
#include <map>

namespace wfc{ namespace core{

struct program_arguments
{
  bool help = false;
  bool usage = false;
  bool generate = false;
  bool version = false;
  bool info = false;
  bool module_list = false;
  bool component_list = false;
  

  bool daemonize = false;
  bool wait_daemonize = false;
  bool coredump = false;
  bool autoup = false;
  bool success_autoup = false;
  time_t autoup_timeout = 0;

  std::string errorstring;
  std::string helpstring;

  std::string user_name;
  std::string working_directory;

  std::string program_name;
  std::string instance_name;
  std::string config_path;
  std::string check_config;
  std::string pid_dir;

  typedef std::map<std::string, std::string> map1;
  typedef std::map<std::string, map1> map2;

  std::vector<std::string> info_options;
  map1 generate_options; // Если не указан -C то индивидуальный запуск
  map2 instance_options;
  map2 startup_options;
  
};

}}
