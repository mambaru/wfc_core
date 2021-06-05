
#include <wfc/iinterface.hpp>
#include "statistics_domain.hpp"
#include <wfc/statistics/statistics.hpp>
#include <fas/utility/ignore_args.hpp>
#include <wrtstat/multi_packer/basic_packer.hpp>
#include <ctime>

namespace wfc{ namespace core{


class statistics_domain::stat_impl
  : public ::wfc::statistics::statistics
  , public ::wfc::iinterface
{
public:
  explicit stat_impl(const wrtstat::wrtstat_options& opt )
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
  workflow_handlers whnd;
  wopt.threads = 1;
  whnd.startup_handler=[this](std::thread::id){ this->reg_thread();};
  whnd.finish_handler=[this](std::thread::id){ this->unreg_thread();};
  for (size_t i = 0 ; i < opt.workers; ++i)
  {
    _stat_list.push_back( std::make_shared<stat_push>( opt ) );
    _stat_list.back()->enable( !this->suspended() );
    _workflow_list.push_back( std::make_shared<workflow_type>(this->global()->io_context, wopt, whnd) );
    _workflow_list.back()->start();
  }

  this->reg_object( "statistics", this->name(), _stat, false);
}

void statistics_domain::initialize()
{
  std::lock_guard<mutex_type> lk(_mutex);
  _targets.reserve(64);
  _targets.clear();
  for ( auto target: this->options().targets )
    _targets.push_back( this->get_target<istatistics>(target) );
}

void statistics_domain::restart()
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

  if ( auto st = this->get_statistics() )
  {
    _multi_push_meter = st->create_time_meter("multi_push.time");
    _multi_count_meter = st->create_size_meter("multi_push.push_count");

    _push_meter = st->create_time_meter("push.time");
    _count_meter = st->create_size_meter("push.values");
  }
}

void statistics_domain::start()
{
  _start_point = std::chrono::system_clock::now();
  this->restart();
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

void statistics_domain::push( push_ptr req, push_handler cb)
{
  if ( this->bad_request(req, cb) )
    return;

  this->push_( std::move(*req) );

  auto res = this->create_response(cb);
  this->send_response( std::move(res), std::move(cb) );
}

void statistics_domain::multi_push( multi_push_ptr req, multi_push_handler cb)
{
  if ( this->bad_request(req, cb) )
    return;

  time_point tm;
  size_point vm;
  if ( auto st = this->get_statistics() )
  {
    tm = _multi_push_meter.create( static_cast<wrtstat::size_type>(1) );
    vm = _multi_count_meter.create( static_cast<wrtstat::value_type>(req->data.size()) );
  }


  std::string err;
  if ( !wrtstat::basic_packer::recompact(req.get(), &err) )
  {
    DOMAIN_LOG_ERROR("Recompact statistics_domain::multi_push: " << err)
    if ( auto res = this->create_response(cb) )
    {
      res->status = false;
      res->error = err;
      this->send_response( std::move(res), std::move(cb) );
    }
  }
  else
  {
    for (push_ptr::element_type& p: req->data )
    {
      this->push_( std::move(p) );
    }

    auto res = this->create_response(cb);
    this->send_response( std::move(res), std::move(cb) );
  }
  fas::ignore_args(tm, vm);
}

void statistics_domain::del( del_ptr req, del_handler cb)
{
  if ( this->bad_request(req, cb) )
    return;

  // TODO: реализовать через выборку
  abort();
  /*
  auto res = this->create_response(cb);

  {
    read_lock<mutex_type> lk(_mutex);
    size_t pos = std::hash<std::string>()(req->name) % _stat_list.size();
    if (auto wf = _workflow_list[pos] )
    {
      auto st = _stat_list[pos];
      auto preq = std::make_shared<wrtstat::request::del>( std::move(*req) );
      wf->post([st, preq](){ st->del( preq->name); }, nullptr);
    }
  }

  if ( res != nullptr )
    res->status = true;

  this->send_response( std::move(res), std::move(cb) );

  for ( auto wt : _targets ) if ( auto t = wt.lock() )
  {
    auto rreq = std::make_unique<wrtstat::request::del>( *req );
    t->del( std::move(rreq), nullptr );
  }*/
}

void statistics_domain::push_( push_ptr::element_type&& req)
{
  time_point tm;
  size_point vm;
  if ( auto st = this->get_statistics() )
  {
    tm = _push_meter.create( static_cast<wrtstat::size_type>(1) );
    vm = _count_meter.create( static_cast<wrtstat::value_type>(req.data.size()) );
  }

  if ( req.ts == 0 )
    req.ts = time(nullptr) * 1000000;

  {
    read_lock<mutex_type> lk(_mutex);
    size_t pos = std::hash<std::string>()(req.name) % _stat_list.size();
    if (auto wf = _workflow_list[pos] )
    {
      auto st = _stat_list[pos];
      auto preq = std::make_shared<push_ptr::element_type>( std::move(req) );
      wf->post([st, preq](){
        st->add( preq->name, *preq);
      }, nullptr);
    }
  }
  fas::ignore_args(tm, vm);
}

template<typename StatPtr>
bool statistics_domain::handler_(StatPtr st, size_t offset, size_t step)
{
  auto opt = this->options();
  if ( !_started )
  {
    auto now = std::chrono::system_clock::now();
    auto diff = std::chrono::duration_cast<std::chrono::milliseconds>( now - _start_point ).count();
    _started = diff > opt.startup_ignore_ms;
    if ( !_started ) return true;
  }

  size_t count = st->aggregators_count();
  for ( size_t i = offset; i < count; i+=step)
  {
    std::string sname = st->get_name(i);
    while (auto ag = st->pop(i) )
    {
      typedef wrtstat::aggregated_data aggregated;
      auto req = std::make_unique<wrtstat::request::push>();
      req->name = sname;
      static_cast<aggregated&>(*req) = std::move(*ag);

      if ( !_targets.empty() )
      {
        for ( size_t j = 1; j < _targets.size(); ++j ) if ( auto t = _targets[j].lock() )
        {
          t->push(std::make_unique<wrtstat::request::push>(*req), nullptr);
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

}}
