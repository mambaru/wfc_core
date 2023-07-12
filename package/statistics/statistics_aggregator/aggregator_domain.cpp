
#include <wfc/iinterface.hpp>
#include "aggregator_domain.hpp"
#include <wfc/statistics/statistics.hpp>
#include <fas/utility/ignore_args.hpp>
#include <wrtstat/multi_packer/basic_packer.hpp>
#include <wrtstat/multi_aggregator/multi_aggregator.hpp>
#include <ctime>

namespace wfc{ namespace core{


class aggregator_domain::stat_impl
  : public wrtstat::multi_aggregator
{
public:

  typedef wrtstat::basic_packer packer_type;
  typedef std::shared_ptr<packer_type> packer_ptr;

  explicit stat_impl(const wrtstat::multi_aggregator_options& opt )
    : multi_aggregator(opt)
  {}

  packer_ptr packer;
};


aggregator_domain::~aggregator_domain()
{
}


void aggregator_domain::reconfigure()
{
  auto opt =  this->options();
  _suspend_push = opt.suspend_push;
  _enabled_packer = !opt.packer.disabled;
  std::lock_guard<mutex_type> lk(_mutex);
  _stat = std::make_shared<stat_impl>( opt );
  _stat->packer = std::make_shared< stat_impl::packer_type >(opt.packer, nullptr);
}

void aggregator_domain::initialize()
{
  std::lock_guard<mutex_type> lk(_mutex);
  _targets.clear();
  for ( auto target: this->options().targets )
    _targets.push_back( this->get_target<istatistics>(target) );
}

void aggregator_domain::restart()
{

  auto opt = this->options();
  if ( auto wf = this->get_workflow() )
  {
    wf->release_timer(_timer_id);
    if ( this->options().pushout_timer_ms!=0 )
    {
      auto st = _stat;
      auto next = std::bind(&aggregator_domain::push_next_, this, std::placeholders::_1);
      _timer_id = wf->create_timer(
        std::chrono::milliseconds( this->options().pushout_timer_ms ),
        [this,  next](){
          this->_stat->pushout(next);
          this->pack_next_mt_(nullptr);
          return true;
        }
      );
    }
    else
      _timer_id = -1;
  }

  if ( auto st = this->get_statistics() )
  {
    _multi_push_meter = st->create_time_meter("multi_push.time");
    _multi_count_meter = st->create_size_meter("multi_push.push_count");

    _push_meter = st->create_time_meter("push.time");
    _count_meter = st->create_size_meter("push.values");
  }
}

void aggregator_domain::start()
{
  // _start_point = std::chrono::system_clock::now();
  this->restart();
}

void aggregator_domain::stop()
{
}

void aggregator_domain::push( push_ptr req, push_handler cb)
{
  if ( this->bad_request(req, cb) )
    return;

  this->push_( std::move(*req) );

  auto res = this->create_response(cb);
  this->send_response( std::move(res), std::move(cb) );
}

void aggregator_domain::multi_push( multi_push_ptr req, multi_push_handler cb)
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
    DOMAIN_LOG_ERROR("Recompact aggregator_domain::multi_push: " << err)
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

void aggregator_domain::del( del_ptr req, del_handler cb)
{
  if ( this->bad_request(req, cb) )
    return;

  auto res = this->create_response(cb);

  for ( const std::string& name : req->names)
  {
    _stat->del( name);
  }

  for ( auto wt : _targets ) if ( auto t = wt.lock() )
  {
    auto rreq = std::make_unique<wrtstat::request::del>( *req );
    t->del( std::move(rreq), nullptr );
  }

  this->send_response( std::move(res), std::move(cb) );
}

void aggregator_domain::push_( push_ptr::element_type&& req)
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

  if ( !_suspend_push )
  {
    _stat->push(req, std::bind(&aggregator_domain::push_next_, this, std::placeholders::_1) );
  }
}

void aggregator_domain::push_next_( push_ptr req)
{
  if ( _enabled_packer )
  {
    this->pack_next_mt_( std::move(req) );
    return;
  }

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

void aggregator_domain::pack_next_mt_( push_ptr req)
{
  std::deque<multi_push_ptr> next_list;
  {
    std::lock_guard<mutex_type> lk(_mutex);
    if ( req!= nullptr )
    {
      _stat->packer->push( std::move(req) );
    }
    while ( auto next_req = _stat->packer->multi_pop() )
      next_list.push_back( std::move(next_req) );
  }

  for ( auto& next_req : next_list)
    this->mulit_push_next_( std::move(next_req) );
}


void aggregator_domain::mulit_push_next_( multi_push_ptr req)
{
  if ( !_targets.empty() )
  {
    for ( size_t j = 1; j < _targets.size(); ++j ) if ( auto t = _targets[j].lock() )
    {
      t->multi_push(std::make_unique<wrtstat::request::multi_push>(*req), nullptr);
    }

    if ( auto t = _targets[0].lock() )
    {
      t->multi_push(std::move(req), nullptr);
    }
  }
}


/*
template<typename StatPtr>
bool aggregator_domain::handler_(StatPtr st, size_t offset, size_t step)
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
*/

}}
