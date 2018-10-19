//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2013-2018
//
// Copyright: See COPYING file that comes with this distribution
//

#pragma once

#include <wfc/domain_object.hpp>
#include <wfc/statistics/istatistics.hpp>
#include "statlog_config.hpp"
#include <string>
#include <memory>

namespace wfc{ namespace core{

using namespace ::wfc::statistics;

class statlog_domain
  : public domain_object<istatistics, statlog_config>
{
public:
  virtual void initialize() override;
  virtual void push( request::push::ptr req, response::push::handler cb ) override;
  virtual void del( request::del::ptr req, response::del::handler cb ) override;
private:
  typedef std::mutex mutex_type;
  std::weak_ptr<istatistics> _target;
  mutex_type _mutex;
  int _id_counter = 0;
  std::map< std::string, int > _legend;
  
};

}}
