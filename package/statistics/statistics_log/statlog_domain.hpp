#pragma once

#include <wfc/domain_object.hpp>
#include <wfc/statistics/ibtp.hpp>
#include "statlog_config.hpp"
#include <string>
#include <memory>

namespace wfc{ namespace core{

using namespace ::wfc::btp;

class statlog_domain
  : public domain_object<ibtp, statlog_config>
{
public:
  virtual void add( request::add::ptr req, response::add::handler cb ) override;
};

}}
