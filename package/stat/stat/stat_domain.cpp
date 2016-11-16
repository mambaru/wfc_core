
#include <wfc/iinterface.hpp>
#include "stat_domain.hpp"

namespace wfc{
 

class stat_domain::impl
  : public stat
{
public:
  impl(const stat_config& /*opt*/ )
    : stat( /*opt*/)
  {}
};


stat_domain::~stat_domain()
{
}

void stat_domain::reconfigure() 
{
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



}
