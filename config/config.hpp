#pragma once
#include <vector>
#include <wfc/core/iconfig.hpp>
#include <wfc/core/global.hpp>
#include <wfc/domain_object.hpp>

#include "configuration.hpp"
#include "config_config.hpp"

namespace wfc{

class config
  :  public ::wfc::domain_object<iconfig, config_config>
{
public:
  virtual ~config();
  config();
  // iconfig
  virtual void reload_and_reconfigure();
  virtual void load_and_parse(std::string path);
  virtual std::string get_config(std::string name);
  std::string generate_and_write_del(std::string type, std::string path);
  virtual bool generate_config( const iconfig::generate_options& go, const std::string& path, std::string& result);

  // core_module
  //void configure(const config_config& conf);
  
  virtual void start(const std::string& arg);
private:
  void _parse_configure(std::string source, std::string strconf, configuration& mainconf);
  std::string _load_from_file(const std::string& path);
  void save_to_file_(const std::string& path, const std::string& strconf);
  
  void _init_timer();
private:
  time_t _config_changed;
  //config_config _conf;
  //std::shared_ptr<global> _global;
  configuration _mainconf;
  std::string _path;
  //std::unique_ptr<callback_owner> _timer_owner;
  //std::unique_ptr<config_timer> _timer;
};

}
