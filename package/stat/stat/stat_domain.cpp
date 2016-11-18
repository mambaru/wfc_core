
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
  //_impl = std::make_shared<impl>(this->options() );
  /*
  auto opt = this->options();
  opt.id = this->name();
  if ( _stat == nullptr )
  {
    _stat = std::make_shared<impl>( this->global()->io_service, opt );
    if ( auto g = this->global() )
      g->registry.set( "stat", this->name(), _stat );
  }
  _stat->reconfigure( opt );
  */
}

void stat_domain::start(const std::string& ) 
{
  /*
  _stat->start();
  */
}

void stat_domain::stop(const std::string& ) 
{
  /*
  if ( auto g = this->global() )
    g->registry.erase("stat", this->name() );
  if ( auto w = _stat )
    w->stop();
  */
}

int stat_domain::reg_name(const std::string& name) 
{
  return _impl->reg_name(name);
}

stat_domain::meter_ptr stat_domain::create_handler(int /*id*/) 
{
  return nullptr;
}

stat_domain::meter_ptr stat_domain::create_handler(const std::string& /*name*/) 
{
  return nullptr;
}




}
