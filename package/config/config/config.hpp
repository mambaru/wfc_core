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
  virtual void ready() override;
  virtual void start() override;
  virtual void stop() override;
  
  // iconfig
  virtual bool reload_and_reconfigure() override;
  virtual bool load_and_configure(std::string path) override;
  virtual bool load_and_check(std::string path) override;
  virtual std::string get_config(std::string name) override;
  virtual bool generate_config( const iconfig::generate_options& go, const std::string& path, std::string& result) override;
  
private:
  bool timer_handler_();
  bool parse_configure_(std::string source, std::string strconf, configuration& mainconf);
  std::string load_from_file_(const std::string& path);
  void save_to_file_(const std::string& path, const std::string& strconf);
  
private:
  workflow::timer_id_t _timer_id;
  time_t _config_changed;
  configuration _mainconf;
  std::string _path;
};

}}
