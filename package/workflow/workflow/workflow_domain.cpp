
#include "workflow_domain.hpp"

namespace wfc{ namespace core{
 
class workflow_domain::impl
  : public iinterface
  , public workflow
{
public:
  explicit impl(io_service_type& io)
    : workflow(io)
  {}

  impl(io_service_type& io, const workflow_options& opt )
    : workflow(io, opt)
  {}
};


workflow_domain::~workflow_domain()
{
}

void workflow_domain::configure() 
{
  _workflow = std::make_shared<impl>( this->global()->io_service, this->options() );
  this->reg_object( "workflow", this->name(), _workflow );
}

void workflow_domain::reconfigure() 
{
  auto opt = this->options();
  opt.id = this->name();
  opt.control_workflow = this->get_common_workflow();
  _workflow->reconfigure(opt);
}

void workflow_domain::initialize()
{
  auto opt = this->statistics_options();
  this->reconfigure();
  if ( auto wrkf = this->get_common_workflow() )
  {
    wrkf->release_timer(_stat_timer);
    if ( opt.queue.empty() || opt.dropped.empty() )
        return;
    if ( auto stat = this->get_statistics() )
    {
      if ( !opt.queue.empty() )
        _meter_size = stat->create_value_meter(this->name() + opt.queue);
      if ( !opt.dropped.empty() )
        _meter_drop = stat->create_value_meter(this->name() + opt.dropped);

      _stat_timer = wrkf->create_timer( std::chrono::milliseconds(opt.interval_ms), [this, stat]()->bool
      {
        size_t dropped = this->_workflow->dropped();
        size_t diffdrop = dropped - this->_dropped;
        this->_meter_size.create( long(this->_workflow->full_size()), 0ul );
        this->_meter_drop.create( 0, diffdrop );
        this->_dropped = dropped;
        return true;
      });
    } // if ( auto stat = this->get_statistics() )
  }
}


void workflow_domain::start()
{
  
  this->restart();
  _workflow->start();
  
}

void workflow_domain::restart() 
{
  auto opt = this->options();
  auto statopt = this->statistics_options();

  //auto g = this->global();
  opt.id = this->name();
  opt.startup_handler = std::bind( &self::reg_thread, this );
  opt.finish_handler = std::bind( &self::unreg_thread, this );

  if ( this->get_statistics() != nullptr )
  {
    std::weak_ptr<workflow_domain> wthis = this->shared_from_this();
    
    bool first = true;
    value_meter proto_time;
    value_meter proto_total;
    auto tcount = std::make_shared< std::atomic<int> >();
    opt.statistics_handler  = 
      [wthis, first, proto_time, proto_total, tcount, statopt]
    (std::thread::id, size_t count, workflow_options::statistics_duration span) mutable
    {
      if ( auto pthis = wthis.lock() )
      {
        if ( auto stat = pthis->get_statistics() )
        {
          if ( first == true )
          {
            first=false;
            int id = tcount->fetch_add(1);
            std::stringstream ss;
            ss << pthis->name() << statopt.thread << id;
            proto_time = stat->create_value_meter( ss.str());
            std::stringstream ss1;
            ss1 << pthis->name() << ".threads";
            proto_total = stat->create_value_meter( ss1.str());
          }
          else
          {
            auto span_mcs = std::chrono::duration_cast<std::chrono::microseconds>(span).count();
            proto_time.create(span_mcs, count );
            proto_total.create(span_mcs, count );
          }
        }
      }
    };
  }
  _workflow->reconfigure( opt );

}

void workflow_domain::stop() 
{
  if ( auto core = this->global()->common_workflow )
    core->release_timer(_stat_timer);
  if ( auto g = this->global() )
    g->registry.erase("workflow", this->name() );
  if ( auto w = _workflow )
    w->stop();
}

}}
