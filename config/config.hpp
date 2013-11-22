#pragma once
#include <vector>
#include <comet/core/iconfig.hpp>
#include <comet/core/imodule.hpp>
#include <comet/inet/imux.hpp>

#include "config_config.hpp"
namespace mamba{ namespace comet{



class config
  : public iconfig
{
  
public:
  
  virtual ~config();

  // iconfig
  virtual void reconfigure();
  virtual bool parse_config(const std::string& path);
  virtual void configure(const std::string& path);
  virtual std::string get_config(const std::string& name);
  virtual std::string generate(const std::string& type);

  // core_module
  void configure(const config_config& conf);
  
private:

};

}}
