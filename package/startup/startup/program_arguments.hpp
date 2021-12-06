//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2013-2018
//
// Copyright: See COPYING file that comes with this distribution
//

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

  // Создать следящий процесс которые автоматически перезапускает демон при падениях
  bool autoup = false;
  // Минимальное время работы дочернего процесса, после которого допустим перезапуск
  // (чтобы не допустить "кувыркания" - непрерывного перезапуска демона из-за ошибок на старте )
  time_t autoup_timeout = 0;
  // Перезапусить процесс даже при успешном завершение (например при kill, kill -s SIGTERM или OOM killer)
  bool success_autoup = false;

  // Время завершения работы в формате "22:00:00" или "1d3m4s" или CRON + working_time
  std::string shutdown_time = "";
  // Время работы в секундах (в том же формате shutdown_time). Если указан shutdown_time, то отсчет после указанного времени
  std::string working_time = "";
  // Не завершать работу, а перезапустить демон если указаны shutdown_time и/или working_time и autoup
  // (нужен мониторящий процесс (указан autoup) иначе не сработает )
  bool restart_by_timer = false;

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
  std::vector<std::string> help_options;
  map1 generate_options; // Если не указан -C то индивидуальный запуск
  map2 object_options;
  map2 startup_options;

};

}}
