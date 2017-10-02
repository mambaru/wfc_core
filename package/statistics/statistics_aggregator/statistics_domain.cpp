
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

  
  if ( auto wf = this->get_workflow() ) 
  {
    wf->release_timer(_stat_wf_id);
    _stat_wf_id = wf->create_timer( 
      std::chrono::milliseconds(opt.aggregate_timeout_ms), 
      std::bind(&statistics_domain::handler_, this, 0, 1) 
    );
    /*
    std::weak_ptr<statistics_domain::impl> wimpl = _impl;
    auto wstatistics = _target;
    std::chrono::system_clock::time_point start = std::chrono::system_clock::now();
    time_t delay = opt.startup_ignore_ms;
    _stat_wf_id = wf->create_timer( 
      std::chrono::milliseconds(opt.aggregate_timeout_ms), 
      [this, wimpl, wstatistics, start, delay]()
    {
      if ( auto pimpl = wimpl.lock() )
      {
        int count = pimpl->count();
        for ( int i = 0; i < count; ++i)
        {
          std::string name = pimpl->get_name(i);
          while (auto ag = pimpl->pop(i) )
          {
            if ( auto pstatistics = wstatistics.lock() )
            {
              auto now = std::chrono::system_clock::now();
              auto diff = std::chrono::duration_cast<std::chrono::milliseconds>( now - start).count();
              
              if ( diff > delay )
              {
                typedef wrtstat::aggregated_data aggregated;
                auto req = std::make_unique<statistics::request::push>();
                req->name = name;
                static_cast<aggregated&>(*req) = std::move(*ag);
                pstatistics->push( std::move(req), nullptr );
              }
            }
          }
        }
      }
      else
        return false;
      return true;
    });
    */
  }
}

void statistics_domain::start() 
{
  _start_point = std::chrono::system_clock::now();
}

void statistics_domain::stop() 
{
  if ( auto wf = this->get_workflow() ) 
    wf->release_timer(_stat_wf_id);

  if ( auto g = this->global() )
  {
    g->registry.erase( "statistics", this->name());
  }
}

void statistics_domain::push( wfc::statistics::request::push::ptr req, wfc::statistics::response::push::handler cb) 
{
  if ( this->bad_request< wfc::statistics::response::push>(req, cb) )
    return;
  
  if ( req->ts == 0 )
    req->ts = time(0) * 1000000;
  
  auto res = this->create_response(cb);
  if ( auto st = _impl )
  {
    if ( auto handler = st->create_aggregator( req->name, req->ts ) )
    {
      if ( res!= nullptr )
        res->result = true;
      handler( *req );
    }
  }
  this->send_response( std::move(res), std::move(cb) );
  
  req->data.clear();
  for ( size_t i = 0; i < _targets.size(); ++i ) if ( auto t = _targets[i].lock() )
  {
    t->push(std::make_unique<wfc::statistics::request::push>(*req), nullptr);
  }

}

}}
