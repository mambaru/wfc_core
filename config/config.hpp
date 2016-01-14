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
  typedef ::wfc::domain_object<iconfig, config_config> super;
public:
  virtual ~config();
  config();
  
  // iconfig
  virtual void reload_and_reconfigure() override;
  virtual void load_and_parse(std::string path) override;
  virtual std::string get_config(std::string name) override;
  virtual bool generate_config( const iconfig::generate_options& go, const std::string& path, std::string& result) override;
  virtual void start(const std::string& arg) override;
  
private:
  void parse_configure_(std::string source, std::string strconf, configuration& mainconf);
  std::string load_from_file_(const std::string& path);
  void save_to_file_(const std::string& path, const std::string& strconf);
  
private:
  time_t _config_changed;
  configuration _mainconf;
  std::string _path;
};

}
