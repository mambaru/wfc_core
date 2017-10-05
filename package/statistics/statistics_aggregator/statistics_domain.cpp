
#include <wfc/iinterface.hpp>
#include "statistics_domain.hpp"
#include <wfc/statistics/statistics.hpp>
#include <ctime>
//#include <wrtstat/wrtstat.hpp>

namespace wfc{ namespace core{
 

class statistics_domain::impl
  : public ::wfc::statistics::statistics
  , public ::wfc::iinterface
{
public:
  impl(const ::wfc::statistics::stat_options& opt )
    : statistics( opt)
  {}
};


statistics_domain::~statistics_domain()
{
}

void statistics_domain::reconfigure_basic()
{
  _impl->enable( !this->suspended()  );
}

void statistics_domain::reconfigure()
{
  _impl = std::make_shared<impl>( this->options() );
  _impl->enable( !this->suspended()  );

  
  if ( auto g = this->global() )
  {
    g->registry.set( "statistics", this->name(), _impl, false);
  }
}

bool statistics_domain::handler_(int offset, int step)
{
  auto opt = this->options();
  if ( !_started )
  {
    auto now = std::chrono::system_clock::now();
    auto diff = std::chrono::duration_cast<std::chrono::milliseconds>( now - _start_point ).count();
    _started = diff > opt.startup_ignore_ms;
    if ( !_started ) return true;
  }
   
  int count = _impl->count();
  for ( int i = offset; i < count; i+=step)
  {
    std::string name = _impl->get_name(i);
    while (auto ag = _impl->pop(i) )
    {
      if ( auto pstatistics = _target.lock() )
      {
        typedef wrtstat::aggregated_data aggregated;
        auto req = std::make_unique<statistics::request::push>();
        req->name = name;
        static_cast<aggregated&>(*req) = std::move(*ag);
        
        for ( size_t i = 0; i < _targets.size(); ++i ) if ( auto t = _targets[i].lock() )
        {
          t->push(std::make_unique<wfc::statistics::request::push>(*req), nullptr);
        }

        pstatistics->push( std::move(req), nullptr );
      }
    }
  }
  return true;
}

void statistics_domain::initialize() 
{
  auto opt = this->options();
  _targets.reserve(64);
  _targets.clear();
  for ( auto target: this->options().targets )
    _targets.push_back( this->get_target<istatistics>(target) );
  _target = this->get_target<istatistics>( opt.target );
}

void statistics_domain::ready() 
{
  auto opt = this->options();
  if ( opt.workers < 1 )
    opt.workers = 1;
  if ( auto wf = this->get_workflow() ) 
  {
    for ( int id : _timers)
      wf->release_timer(id);
    _timers.clear();
    for (int i = 0; i < opt.workers; ++i)
    {
      
      int id = wf->create_timer( 
        std::chrono::milliseconds(opt.aggregate_timeout_ms), 
        std::bind(&statistics_domain::handler_, this, i, opt.workers) 
      );
      _timers.push_back(id);
    }

  }
  
  if ( auto st = this->get_statistics() )
  {
    _push_meter = st->create_time_prototype("push.time");
    _count_meter = st->create_size_prototype("push.values");
  }
}

void statistics_domain::start() 
{
  _start_point = std::chrono::system_clock::now();
  this->ready();
}

void statistics_domain::stop() 
{
  if ( auto wf = this->get_workflow() ) 
  {
    for ( int id : _timers)
      wf->release_timer(id);
    _timers.clear();
  }

  if ( auto g = this->global() )
  {
    g->registry.erase( "statistics", this->name());
  }
}

void statistics_domain::push( wfc::statistics::request::push::ptr req, wfc::statistics::response::push::handler cb) 
{
  if ( this->bad_request(req, cb) )
    return;
  
  time_meter_ptr tm;
  size_meter_ptr vm;
  if ( auto st = this->get_statistics() )
  {
    tm = st->create_meter(_push_meter, 1);
    vm = st->create_meter(_count_meter, req->data.size() );
  }
  
  if ( req->ts == 0 )
    req->ts = time(0) * 1000000;
  
  auto res = this->create_response(cb);
  if ( auto st = _impl )
  {
    if ( auto handler = st->create_aggregator( req->name, req->ts ) )
    {
      if ( res!= nullptr )
        res->status = true;
      handler( *req );
    }
  }
  this->send_response( std::move(res), std::move(cb) );
  /*
  //!! req->data.clear();
  if ( !_targets.empty() )
  {
    for ( size_t i = 1; i < _targets.size(); ++i ) if ( auto t = _targets[i].lock() )
    {
      t->push(std::make_unique<wfc::statistics::request::push>(*req), nullptr);
    }
    if ( auto t = _targets[0].lock() )
      t->push( std::move(req), nullptr);
  }*/
}

void statistics_domain::del( wfc::statistics::request::del::ptr req, wfc::statistics::response::del::handler cb) 
{
  if ( this->bad_request(req, cb) )
    return;

  auto res = this->create_response(cb);
  if ( auto st = _impl )
  {
    res->status = st->del(req->name);
  }
  this->send_response( std::move(res), std::move(cb) );
  
  for ( auto wt : _targets ) if ( auto t = wt.lock() )
  {
    auto rreq = std::make_unique<wfc::statistics::request::del>( *req );
    t->del( std::move(rreq), nullptr );
  }
  if ( auto t = _target.lock() )
  {
    t->del( std::move(req), nullptr );
  }
}

}}
