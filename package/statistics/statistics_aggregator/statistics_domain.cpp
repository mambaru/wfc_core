
#include <wfc/iinterface.hpp>
#include "statistics_domain.hpp"
#include <wfc/statistics/statistics.hpp>
#include <ctime>

namespace wfc{ namespace core{
 

class statistics_domain::stat_impl
  : public ::wfc::statistics::statistics
  , public ::wfc::iinterface
{
public:
  stat_impl(const ::wfc::statistics::stat_options& opt )
    : statistics( opt)
  {}
};


statistics_domain::~statistics_domain()
{
}

void statistics_domain::reconfigure_basic()
{
  std::lock_guard<mutex_type> lk(_mutex);
  _stat->enable( !this->suspended()  );
  for (auto st : _stat_list)
    st->enable( !this->suspended()  );
}

void statistics_domain::reconfigure()
{
  auto opt =  this->options();
  std::lock_guard<mutex_type> lk(_mutex);
  _stat = std::make_shared<stat_impl>( opt );
  _stat->enable( !this->suspended()  );
  _stat_list.clear();
  _stat_list.reserve(opt.hash_size);
  for (size_t i = 0 ; i < opt.hash_size; ++i)
  {
    _stat_list.push_back( std::make_shared<stat_impl>( opt ) );
    _stat_list.back()->enable( !this->suspended() );
  }
  
  if ( auto g = this->global() )
  {
    g->registry.set( "statistics", this->name(), _stat, false);
  }
}

void statistics_domain::initialize() 
{
  auto opt = this->options();
  std::lock_guard<mutex_type> lk(_mutex);
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
  stat_list stats = _stat_list;
  stats.insert(stats.begin(), _stat);
  if ( auto wf = this->get_workflow() ) 
  {
    for ( int id : _timers)
      wf->release_timer(id);
    _timers.clear();
    for ( auto st : stats )
    {
      for (int i = 0; i < opt.workers; ++i)
      {
        int id = wf->create_timer( 
          std::chrono::milliseconds(opt.aggregate_timeout_ms), 
          std::bind(&statistics_domain::handler_, this, st, i, opt.workers) 
        );
        _timers.push_back(id);
      }
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
  if ( auto st = this->get_stat_(req->name) )
  {
    bool status = st->add( req->name, *req);
    if ( res!= nullptr )
      res->status = status;
  }
  this->send_response( std::move(res), std::move(cb) );
}

void statistics_domain::del( wfc::statistics::request::del::ptr req, wfc::statistics::response::del::handler cb) 
{
  if ( this->bad_request(req, cb) )
    return;

  auto res = this->create_response(cb);
  if ( auto st = this->get_stat_(req->name) )
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


bool statistics_domain::handler_(stat_ptr st, int offset, int step)
{
  auto opt = this->options();
  if ( !_started )
  {
    auto now = std::chrono::system_clock::now();
    auto diff = std::chrono::duration_cast<std::chrono::milliseconds>( now - _start_point ).count();
    _started = diff > opt.startup_ignore_ms;
    if ( !_started ) return true;
  }
   
  int count = st->count();
  for ( int i = offset; i < count; i+=step)
  {
    std::string name = st->get_name(i);
    while (auto ag = st->pop(i) )
    {
      typedef wrtstat::aggregated_data aggregated;
      auto req = std::make_unique<statistics::request::push>();
      req->name = name;
      static_cast<aggregated&>(*req) = std::move(*ag);
        
      for ( size_t i = 0; i < _targets.size(); ++i ) if ( auto t = _targets[i].lock() )
      {
        t->push(std::make_unique<wfc::statistics::request::push>(*req), nullptr);
      }

      if ( auto pstat = _target.lock() )
        pstat->push( std::move(req), nullptr );
    }
  }
  return true;
}

statistics_domain::stat_ptr statistics_domain::get_stat_(const std::string& name)
{
  read_lock<mutex_type> lk(_mutex);
  if ( _stat_list.empty() )
    return _stat;
  size_t pos = std::hash<std::string>()(name) % _stat_list.size();
  return _stat_list[pos];
}


}}
