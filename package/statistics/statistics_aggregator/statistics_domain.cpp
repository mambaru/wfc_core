
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
  stat_impl(const wrtstat::wrtstat_options& opt )
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
  _suspend_push = opt.suspend_push;
  std::lock_guard<mutex_type> lk(_mutex);
  _stat = std::make_shared<stat_impl>( opt );
  _stat->enable( !this->suspended()  );
  _stat_list.clear();
  for (auto w : _workflow_list)
    w->stop();
  _workflow_list.clear();
  _stat_list.reserve(opt.workers);
  _workflow_list.reserve(opt.workers);
  workflow_options wopt;
  wopt.threads = 1;
  wopt.startup_handler=[this](std::thread::id){ this->reg_thread();};
  wopt.finish_handler=[this](std::thread::id){ this->unreg_thread();};
  for (size_t i = 0 ; i < opt.workers; ++i)
  {
    _stat_list.push_back( std::make_shared<stat_push>( opt ) );
    _stat_list.back()->enable( !this->suspended() );
    _workflow_list.push_back( std::make_shared<workflow_type>(wopt) );
    _workflow_list.back()->start();
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
  //_target = this->get_target<istatistics>( opt.target );
}

void statistics_domain::ready() 
{
  auto opt = this->options();
  if ( auto wf = this->get_workflow() )
  {
    auto st = _stat;
    wf->release_timer(_timer_id);
    _timer_id = wf->create_timer( 
      std::chrono::milliseconds(opt.aggregate_timeout_ms), 
      std::bind(&statistics_domain::handler_<stat_ptr>, this, st, 0, 1) 
    );
  }
  
  for (size_t i = 0 ; i < opt.workers; ++i)
  {
    if (auto wf = _workflow_list[i] )
    {
      auto st = _stat_list[i];
      wf->create_timer( 
        std::chrono::milliseconds(opt.aggregate_timeout_ms), 
        std::bind(&statistics_domain::handler_<stat_push_ptr>, this, st, i, opt.workers) 
      );
    }
  }
  
  
  /*
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
  */
  
  if ( auto st = this->get_statistics() )
  {
    _push_meter = st->create_time_factory("push.time");
    _count_meter = st->create_size_factory("push.values");
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
  
  time_meter tm;
  size_meter vm;
  if ( auto st = this->get_statistics() )
  {
    tm = _push_meter.create(1);
    vm = _count_meter.create(req->data.size() );
  }
  
  if ( req->ts == 0 )
    req->ts = time(0) * 1000000;
  
  auto res = this->create_response(cb);
  {
    read_lock<mutex_type> lk(_mutex);
    size_t pos = std::hash<std::string>()(req->name) % _stat_list.size();
    if (auto wf = _workflow_list[pos] )
    {
      auto st = _stat_list[pos];
      auto preq = std::make_shared<wfc::statistics::request::push>( std::move(*req) );
      wf->post([st, preq](){ 
        st->add( preq->name, *preq); 
      }, nullptr);
    }
  }
  /*
  if ( auto st = this->get_stat_(req->name) )
  {
    bool status = _suspend_push ? false : st->add( req->name, *req);
    if ( res!= nullptr )
      res->status = status;
  }
  */
  this->send_response( std::move(res), std::move(cb) );
}

void statistics_domain::del( wfc::statistics::request::del::ptr req, wfc::statistics::response::del::handler cb) 
{
  if ( this->bad_request(req, cb) )
    return;

  auto res = this->create_response(cb);
  
  {
    read_lock<mutex_type> lk(_mutex);
    size_t pos = std::hash<std::string>()(req->name) % _stat_list.size();
    if (auto wf = _workflow_list[pos] )
    {
      auto st = _stat_list[pos];
      auto preq = std::make_shared<wfc::statistics::request::del>( std::move(*req) );
      wf->post([st, preq](){ st->del( preq->name); }, nullptr);
    }
  }

  if ( res != nullptr )
    res->status = true;
  /*
  if ( auto st = this->get_stat_(req->name) )
  {
    res->status = st->del(req->name);
  }
  */
  this->send_response( std::move(res), std::move(cb) );
  
  for ( auto wt : _targets ) if ( auto t = wt.lock() )
  {
    auto rreq = std::make_unique<wfc::statistics::request::del>( *req );
    t->del( std::move(rreq), nullptr );
  }
  /*if ( auto t = _target.lock() )
  {
    t->del( std::move(req), nullptr );
  }*/
}


template<typename StatPtr>
bool statistics_domain::handler_(StatPtr st, int offset, int step)
{
  auto opt = this->options();
  if ( !_started )
  {
    auto now = std::chrono::system_clock::now();
    auto diff = std::chrono::duration_cast<std::chrono::milliseconds>( now - _start_point ).count();
    _started = diff > opt.startup_ignore_ms;
    if ( !_started ) return true;
  }
   
  int count = st->aggregators_count();
  for ( int i = offset; i < count; i+=step)
  {
    std::string name = st->get_name(i);
    while (auto ag = st->pop(i) )
    {
      typedef wrtstat::aggregated_data aggregated;
      auto req = std::make_unique<statistics::request::push>();
      req->name = name;
      static_cast<aggregated&>(*req) = std::move(*ag);
        
      if ( !_targets.empty() )
      {
        for ( size_t i = 1; i < _targets.size(); ++i ) if ( auto t = _targets[i].lock() )
        {
          std::cout << "TEST " << req->name << " ts " << req->ts << std::endl;
          t->push(std::make_unique<wfc::statistics::request::push>(*req), nullptr);
        }
        
        if ( auto t = _targets[0].lock() )
        {
          t->push(std::move(req), nullptr);
        }
      }
    }
  }
  return true;
}

/*
statistics_domain::stat_ptr statistics_domain::get_stat_(const std::string& name)
{
  read_lock<mutex_type> lk(_mutex);
  if ( _stat_list.empty() )
    return _stat;
  size_t pos = std::hash<std::string>()(name) % _stat_list.size();
  return _stat_list[pos];
}
*/

/*
statistics_domain::workflow_ptr statistics_domain::get_workflow_(const std::string& name)
{
  read_lock<mutex_type> lk(_mutex);
  if ( _wor_list.empty() )
    return _stat;
  size_t pos = std::hash<std::string>()(name) % _stat_list.size();
  return _stat_list[pos];
}
*/


}}
