#pragma once

#include <wfc/domain_object.hpp>
#include <wfc/statistics/ibtp.hpp>
#include "system_statistics_config.hpp"
#include <string>
#include <memory>

namespace wfc{  namespace core{

class system_statistics_domain
  : public domain_object<iinterface, statistics_config>
{
  class impl;
public:
  virtual void configure() override;
  virtual void initialize() override;
private:
  std::shared_ptr<impl> _impl;
};

}}
