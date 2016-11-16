#pragma once

#include <wfc/domain_object.hpp>
#include <wfc/stat/stat.hpp>
#include "stat_config.hpp"
#include <string>
#include <memory>

namespace wfc{

class stat_domain
  : public domain_object<iinterface, stat_config>
{
  class impl;
public:
  virtual ~stat_domain();
  virtual void reconfigure() override;
  virtual void start(const std::string& ) override;
  virtual void stop(const std::string& ) override;
private:
  std::shared_ptr<impl> _stat;
};

}
