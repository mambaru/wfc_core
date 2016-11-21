
#include <wfc/iinterface.hpp>
#include "stat_domain.hpp"
#include <wfc/stat/stat.hpp>
//#include <wrtstat/wrtstat.hpp>

namespace wfc{
 

class stat_domain::impl
  : public ::wfc::stat
{
public:
  impl(const stat_config& opt )
    : stat( opt)
  {}
};


stat_domain::~stat_domain()
{
}

void stat_domain::reconfigure() 
{
  _impl = std::make_shared<impl>( this->options() );
}



int stat_domain::reg_name(const std::string& name) 
{
  return _impl->reg_name(name);
}

stat_domain::meter_ptr stat_domain::create_handler(int id) 
{
  return _impl->create_handler(id);
}

stat_domain::meter_ptr stat_domain::create_handler(const std::string& name) 
{
  return _impl->create_handler(name);
}




}
