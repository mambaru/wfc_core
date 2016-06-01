
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
  this->stop("");
}

void workflow_domain::create() 
{
  _workflow = std::make_shared<impl>( this->global()->io_service, this->options() );
  this->global()->registry.set( "workflow", this->name(), _workflow );
}

void workflow_domain::reconfigure() 
{
  _workflow->reconfigure( this->options() );
}

void workflow_domain::start(const std::string& ) 
{
  _workflow->start();
}

void workflow_domain::stop(const std::string& ) 
{
  this->global()->registry.erase("workflow", this->name() );
  _workflow->stop();
}



}
