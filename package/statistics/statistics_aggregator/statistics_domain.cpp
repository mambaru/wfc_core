
#include <wfc/iinterface.hpp>
#include "statistics_domain.hpp"
#include <wfc/statistics/statistics.hpp>
#include <ctime>
//#include <wrtstat/wrtstat.hpp>

namespace wfc{ namespace core{
 

class statistics_domain::impl
  : public ::wfc::statistics
  , public ::wfc::iinterface
{
public:
  impl(const ::wfc::stat_options& opt )
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
  _impl = std::make_shared<impl>( this->statistics_options() );
  _impl->enable( !this->suspended()  );

  
  if ( auto g = this->global() )
  {
    g->registry.set( "statistics", this->name(), _impl, false);
  }
}

void statistics_domain::initialize() 
{
  auto opt = this->options();
  _wbtp = this->get_target<ibtp>( opt.target );

  if ( auto wf = this->get_workflow() ) 
  {
    wf->release_timer(_stat_wf_id);

    std::weak_ptr<statistics_domain::impl> wimpl = _impl;
    auto wbtp = _wbtp;
    auto start = std::chrono::system_clock::now();
    time_t delay = opt.startup_ignore_ms;
    _stat_wf_id = wf->create_timer( 
      std::chrono::milliseconds(opt.aggregate_timeout_ms), 
      [this, wimpl, wbtp, start, delay]()
    {
      if ( auto pimpl = wimpl.lock() )
      {
        int count = pimpl->count();
        for ( int i = 0; i < count; ++i)
        {
          std::string name = pimpl->get_name(i);
          while (auto ag = pimpl->pop(i) )
          {
            if ( auto pbtp = wbtp.lock() )
            {
              auto now = std::chrono::system_clock::now();
              auto diff = std::chrono::duration_cast<std::chrono::milliseconds>( now - start).count();
              
              if ( diff > delay )
              {
                typedef wrtstat::aggregated_data aggregated;
                auto req = std::make_unique<btp::request::add>();
                req->name = name;
                static_cast<aggregated&>(*req) = std::move(*ag);
                pbtp->add( std::move(req), nullptr );
              }
            }
          }
        }
      }
      else
        return false;
      return true;
    });
  }
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

void statistics_domain::add( wfc::btp::request::add::ptr req, wfc::btp::response::add::handler cb) 
{
  if ( this->bad_request< wfc::btp::response::add>(req, cb) )
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
}

}}
