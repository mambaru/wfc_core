//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2013-2018
//
// Copyright: See COPYING file that comes with this distribution
//

#pragma once

#include "configuration.hpp"
#include "config_config.hpp"

#include <wfc/core/iconfig.hpp>
#include <wfc/domain_object.hpp>

#include <string>

namespace wfc{ namespace core{

class config
  :  public ::wfc::domain_object<iconfig, config_config, nostat>
{
public:
  virtual ~config();
  config();

  //domain_object
  virtual void restart() override;
  virtual void start() override;
  virtual void stop() override;

  // iconfig
  virtual bool reload_and_reconfigure() override;
  virtual bool load_and_configure(std::string path) override;
  virtual bool load_and_check(std::string path) override;
  virtual std::string get_config(std::string component_name) override;
  virtual bool generate_config( const iconfig::generate_options& go, const std::string& path, std::string& result) override;

private:
  bool timer_handler_();
  bool parse_configure_(const std::string& source, const std::string& confstr, configuration& mainconf);
  std::string load_from_file_(const std::string& path, bool is_reload);
  void save_to_file_(const std::string& path, const std::string& confstr);

private:
  wflow::workflow::timer_id_t _timer_id;
  configuration _mainconf;
  std::string _path;
  std::map<std::string, time_t> _changed_map;
};

}}
