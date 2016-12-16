#pragma once

#include <wfc/domain_object.hpp>
#include <wfc/statistics/ibtp.hpp>
#include "system_statistics_config.hpp"
#include <string>
#include <memory>

namespace wfc{  namespace core{

class system_statistics_domain
  : public domain_object<iinterface, system_statistics_config>
{
  //struct protostat;
public:
  virtual void configure() override;
  virtual void ready() override;
  virtual void stop(const std::string&) override;
private:
  timer_id_t _timer_id = -1;
  timer_id_t _timer_id2 = -1;
};

}}
