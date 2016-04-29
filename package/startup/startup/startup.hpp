//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2013-2015
//
// Copyright: See COPYING file that comes with this distribution
//

#pragma once

#include "program_arguments.hpp"
#include "startup_config.hpp"
#include <wfc/core/istartup.hpp>
#include <wfc/domain_object.hpp>
#include <string>

namespace wfc{

class idle_timer;
  
class startup_domain
  : public ::wfc::domain_object<istartup, startup_config>
  , public std::enable_shared_from_this<startup_domain>
{
public:
  virtual ~startup_domain();
  virtual bool startup( int argc, char* argv[], std::string helpstring) override;
private:
  bool perform_start_();
  bool generate_();
  void show_usage_();
  void show_info_(const std::string& name);
  void show_build_info_(std::shared_ptr<ibuild_info> b, bool shortinfo);
private:
  program_arguments _pa;
};

}
