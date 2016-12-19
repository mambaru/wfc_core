
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
  if ( auto core = this->global()->workflow )
  {
    core->release_timer(_stat_timer);
    if ( auto stat = this->get_statistics() )
    {
      if ( !opt.stat.queue.empty() )
        _meter_size = stat->create_value_prototype(this->name() + opt.stat.queue);
      if ( !opt.stat.dropped.empty() )
        _meter_drop = stat->create_value_prototype(this->name() + opt.stat.dropped);

      if ( !opt.stat.thread.empty() )
      {
        _meters_threads.clear();
        _counters.clear();
        for ( int i = 0; i < opt.threads; i++ )
        {
          std::stringstream ss;
          ss << this->name() << opt.stat.thread << i;
          _meters_threads.push_back( stat->create_value_prototype(ss.str()) );
          _counters.push_back(0);
        }
      }

      if ( _meter_size == nullptr && _meter_drop==nullptr && _meters_threads.empty() )
        return;

      _stat_timer = core->create_timer( std::chrono::milliseconds(opt.stat.interval_ms), [this, stat]()->bool
      {
        size_t dropped = this->_workflow->dropped() - this->_dropped;
        stat->create_meter( this->_meter_size, this->_workflow->queue_size(), 0 );
        stat->create_meter( this->_meter_drop, 0, dropped );
        
        if ( !this->_counters.empty() )
        {
          size_t threads = this->_workflow->manager()->get_threads();
          for ( size_t i =0 ; i < threads; ++i )
          {
            size_t counter = this->_workflow->manager()->get_counter(i) - this->_counters[i];
            this->_counters[i] = this->_workflow->manager()->get_counter(i);
            stat->create_meter( _meters_threads[i], 0, counter );
          }
        }
        
        return true;
      });
    } // if ( auto stat = this->get_statistics() )
  }
}


void workflow_domain::start()
{
  _workflow->start();
}

void workflow_domain::stop() 
{
  if ( auto core = this->global()->workflow )
    core->release_timer(_stat_timer);
  if ( auto g = this->global() )
    g->registry.erase("workflow", this->name() );
  if ( auto w = _workflow )
    w->stop();
}

}}
