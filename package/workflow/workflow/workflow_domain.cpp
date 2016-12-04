
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
  auto opt = this->options();
  opt.id = this->name();
  if ( _workflow == nullptr )
  {
    _workflow = std::make_shared<impl>( this->global()->io_service, opt );
    if ( auto g = this->global() )
      g->registry.set( "workflow", this->name(), _workflow );
  }
  _workflow->reconfigure( opt );
}

void workflow_domain::initialize()
{
  if ( auto core = this->get_workflow() )
  {
    core->release_timer(_stat_timer);
    _meter_size = this->create_meter_prototype("", this->name() + ".queue_size");
    _meter_drop = this->create_meter_prototype("", this->name() + ".dropped");

    if ( _meter_size == nullptr )
      return;

    _stat_timer = core->create_timer( std::chrono::seconds(1), [this]()->bool
    {
      this->create_meter(_meter_size, _workflow->queue_size() );
      this->create_meter(_meter_drop, _workflow->dropped() );
      return true;
    });
  }
}


void workflow_domain::start(const std::string& ) 
{
  _workflow->start();
}

void workflow_domain::stop(const std::string& ) 
{
  if ( auto core = this->get_workflow() )
    core->release_timer(_stat_timer);
  if ( auto g = this->global() )
    g->registry.erase("workflow", this->name() );
  if ( auto w = _workflow )
    w->stop();
}



}
