#pragma once
#include <vector>
#include <comet/core/iconfig.hpp>
#include <comet/core/imodule.hpp>
#include <comet/inet/imux.hpp>

#include "configuration.hpp"
#include "config_config.hpp"
namespace mamba{ namespace comet{



class config
  : public iconfig
{
  
public:
  
  virtual ~config();

  config(std::shared_ptr<global> gl);
  // iconfig
  virtual void reconfigure();
  virtual bool parse_config(const std::string& path);
  virtual void configure(const std::string& path);
  virtual std::string get_config(const std::string& name);
  virtual std::string generate(const std::string& type, const std::string& path);
  // core_module
  void configure(const config_config& conf);
private:
  std::string _load_from_file(const std::string& path);
  void _save_to_file(const std::string& path, const std::string& strconf);
private:
  std::shared_ptr<global> _global;
  configuration _mainconf;
  std::string _path;
};

}}
