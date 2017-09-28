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
  virtual void initialize() override;
  virtual void add( request::add::ptr req, response::add::handler cb ) override;
private:
  typedef std::mutex mutex_type;
  std::weak_ptr<ibtp> _target;
  mutex_type _mutex;
  int _id_counter = 0;
  std::map< std::string, int > _legend;
  
};

}}
