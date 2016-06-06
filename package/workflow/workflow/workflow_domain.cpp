
#include "workflow_domain.hpp"

namespace wfc{
 

class workflow_domain::impl
  : public iinterface
  , public workflow
{
public:
  impl(io_service_type& io, const workflow_options& opt )
    : workflow(io, opt)
  {}
};


workflow_domain::~workflow_domain()
{
}

void workflow_domain::reconfigure() 
{
  if ( _workflow == nullptr )
  {
    _workflow = std::make_shared<impl>( this->global()->io_service, this->options() );
    if ( auto g = this->global() )
      g->registry.set( "workflow", this->name(), _workflow );
  }
  _workflow->reconfigure( this->options() );
}

void workflow_domain::start(const std::string& ) 
{
  _workflow->start();
}

void workflow_domain::stop(const std::string& ) 
{
  if ( auto g = this->global() )
    g->registry.erase("workflow", this->name() );
  if ( auto w = _workflow )
    w->stop();
}



}
