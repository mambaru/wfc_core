
#include "workflow_domain.hpp"

namespace wfc{ namespace core{
 
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
  auto opt = this->options();
  if ( auto core = this->get_workflow() )
  {
    core->release_timer(_stat_timer);
    if ( !opt.stat_names.queue.empty() )
      _meter_size = this->create_meter_prototype("", this->name() + opt.stat_names.queue);
    if ( !opt.stat_names.dropped.empty() )
      _meter_drop = this->create_meter_prototype("", this->name() + opt.stat_names.dropped);
    
    
    if ( !opt.stat_names.thread.empty() && this->get_statistics()!=nullptr )
    {
      _meters_threads.clear();
      _counters.clear();
      for ( int i = 0; i < opt.threads; i++ )
      {
        std::stringstream ss;
        ss << this->name() << opt.stat_names.thread << i;
        _meters_threads.push_back( this->create_meter_prototype("", ss.str()) );
        _counters.push_back(0);
      }
    }
    

    if ( _meter_size == nullptr && _meter_drop==nullptr && _meters_threads.empty() )
      return;
    

    _stat_timer = core->create_timer( std::chrono::seconds(1), [this]()->bool
    {
      size_t dropped = this->_workflow->dropped() - this->_dropped;
      this->create_meter( this->_meter_size, this->_workflow->queue_size(), 0 );
      this->create_meter( this->_meter_drop, 0, dropped );
      
      if ( !this->_counters.empty() )
      {
        size_t threads = this->_workflow->get()->get_threads();
        for ( size_t i =0 ; i < threads; ++i )
        {
          size_t counter = this->_workflow->get()->get_counter(i) - this->_counters[i];
          this->_counters[i] = this->_workflow->get()->get_counter(i);
          this->create_meter( _meters_threads[i], 0, counter );
        }
      }
      
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

}}
