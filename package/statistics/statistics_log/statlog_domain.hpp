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
  virtual void configure() override;
  virtual void reconfigure() override;

  virtual void initialize() override;
  virtual void push( push_ptr req, push_handler cb ) override;
  virtual void multi_push( multi_push_ptr req, multi_push_handler cb ) override;
  virtual void del( del_ptr req, del_handler cb ) override;
private:
  void configure_();
  void push_log_( const push_ptr::element_type& ag );
  
  template<typename T>
  void write_field_( std::stringstream& ss, const std::string& logname, const T& field ) const;

private:
  typedef std::mutex mutex_type;
  std::weak_ptr<istatistics> _target;
  table_format_options _table_format;
  mutex_type _mutex;
  std::set< std::string> _legend;
};

}}
