
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

void workflow_domain::configure() 
{
  this->reconfigure();
}

void workflow_domain::reconfigure() 
{
  auto name = this->name();
  auto opt = this->options();
  opt.id = name;
  if ( _workflow == nullptr )
  {
    _workflow = std::make_shared<impl>( this->global()->io_service, opt );
    if ( auto g = this->global() )
      g->registry.set( "workflow", name, _workflow );
    // Стартуем сразу, чтобы запросы не терять не 
    //_workflow->start();
    //this->ready();
    
  }
  else
    _workflow->reconfigure(opt);

}

void workflow_domain::initialize()
{
  auto opt = this->statistics_options();
  if ( auto core = this->global()->workflow )
  {
    core->release_timer(_stat_timer);
    if ( auto stat = this->get_statistics() )
    {
      if ( !opt.queue.empty() )
        _meter_size = stat->create_value_prototype(this->name() + opt.queue);
      if ( !opt.dropped.empty() )
        _meter_drop = stat->create_value_prototype(this->name() + opt.dropped);

      /*
      if ( !opt.stat.thread.empty() )
      {
        _meters_threads.clear();
        _counters.clear();
        for ( int i = 0; i < opt.threads; i++ )
        {
          std::stringstream ss;
          ss << this->name() << opt.stat.thread << i;
          //_meters_threads.push_back( stat->create_value_prototype(ss.str()) );
          _counters.push_back(0);
        }
      }*/

      if ( _meter_size == nullptr && _meter_drop==nullptr /*&& _meters_threads.empty()*/ )
        return;

      _stat_timer = core->create_timer( std::chrono::milliseconds(opt.interval_ms), [this, stat]()->bool
      {
        size_t dropped = this->_workflow->dropped();
        size_t diffdrop = dropped - this->_dropped;
        stat->create_meter( this->_meter_size, this->_workflow->queue_size(), 0 );
        stat->create_meter( this->_meter_drop, 0, diffdrop );
        this->_dropped = dropped;
        
        /*
        if ( !this->_counters.empty() )
        {
          size_t threads = this->_workflow->manager()->get_threads();
          for ( size_t i =0 ; i < threads; ++i )
          {
            //size_t counter = this->_workflow->manager()->get_counter(i) - this->_counters[i];
            this->_counters[i] = this->_workflow->manager()->get_counter(i);
            //stat->create_meter( _meters_threads[i], 0, counter );
          }
        }
        */
        return true;
      });
    } // if ( auto stat = this->get_statistics() )
  }
}


void workflow_domain::start()
{
  
  this->ready();
  _workflow->start();
  
}

void workflow_domain::ready() 
{
  auto opt = this->options();
  auto statopt = this->statistics_options();

  auto g = this->global();
  auto name = this->name();
  opt.id = name;

  g->threads.set_reg_cpu(name, opt.cpu);
  opt.startup_handler = [g, name](std::thread::id id)
  {
    COMMON_LOG_MESSAGE( "Start workflow thread " << id )
    g->threads.reg_thread(name);
  };

  opt.finish_handler = [g](std::thread::id id)
  {
    COMMON_LOG_MESSAGE( "Finish workflow thread " << id )
    g->threads.unreg_thread();
  }; 

  if ( this->get_statistics() != nullptr )
  {
    std::weak_ptr<workflow_domain> wthis = this->shared_from_this();
    value_meter_ptr proto_time;
    value_meter_ptr proto_total;
    /*value_meter_ptr proto_count;*/
    auto tcount = std::make_shared< std::atomic<int> >();
    opt.statistics_handler  = [wthis, proto_time, proto_total, tcount, statopt](std::thread::id, size_t count, workflow_options::statistics_duration span) mutable
    {
      if ( auto pthis = wthis.lock() )
      {
        if ( auto stat = pthis->get_statistics() )
        {
          if ( proto_time == nullptr )
          {
            size_t id = tcount->fetch_add(1);
            std::stringstream ss;
            ss << pthis->name() << statopt.thread << id;
            proto_time = stat->create_value_prototype( ss.str());
            std::stringstream ss1;
            ss1 << pthis->name() << ".threads";
            proto_total = stat->create_value_prototype( ss1.str());
          }
          else
          {
            auto span_mcs = std::chrono::duration_cast<std::chrono::microseconds>(span).count();
            stat->create_meter(proto_time, span_mcs, count );
            stat->create_meter(proto_total, span_mcs, count );
          }
        }
      }
    };
  }
  _workflow->reconfigure( opt );

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
