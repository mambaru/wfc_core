#pragma once

#include <wfc/domain_object.hpp>
#include <wfc/stat/istat.hpp>
#include "stat_config.hpp"
#include <string>
#include <memory>

namespace wfc{

class stat_domain
  : public domain_object<istat, stat_config>
{
  class impl;
public:
  virtual ~stat_domain();
  virtual void reconfigure() override;
  virtual void start(const std::string& ) override;
  virtual void stop(const std::string& ) override;
  
  virtual       int reg_name(const std::string& name) override;
  virtual meter_ptr create_handler(int id) override;
  virtual meter_ptr create_handler(const std::string& name) override;

private:
  std::shared_ptr<impl> _impl;
};

}
