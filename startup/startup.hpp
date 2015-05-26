//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2013-2015
//
// Copyright: See COPYING file that comes with this distribution
//

#pragma once

#include "domain/program_arguments.hpp"
#include "startup_config.hpp"
#include <wfc/core/istartup.hpp>
#include <wfc/domain_object.hpp>
#include <string>

namespace wfc{

class idle_timer;
  
class startup_domain
  : public ::wfc::domain_object<istartup, startup_config>
{
public:
  virtual ~startup_domain();
  virtual bool startup( int argc, char* argv[] );
private:
  bool startup_( int argc, char** argv);
  bool generate_(const program_arguments& pa);
  void generate_( const std::string& type, const std::string& path );
  void show_usage_();
  void show_help_();
  void show_info_();
  void show_module_info_(const std::string& module_name);
private:
};

}
