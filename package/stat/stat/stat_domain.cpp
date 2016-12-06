
#include <wfc/iinterface.hpp>
#include "stat_domain.hpp"
#include <wfc/statistics/statistics.hpp>
//#include <wrtstat/wrtstat.hpp>

namespace wfc{
 

class stat_domain::impl
  : public ::wfc::statistics
  , public ::wfc::iinterface
{
public:
  impl(const stat_config& opt )
    : statistics( opt)
  {}
};


stat_domain::~stat_domain()
{
}

void stat_domain::reconfigure_basic()
{
  _impl->enable( !this->suspended()  );
}

void stat_domain::reconfigure()
{
  _impl = std::make_shared<impl>( this->options() );
  _impl->enable( !this->suspended()  );

  if ( auto wf = this->get_workflow() ) 
    wf->release_timer(_stat_wf_id);
  
  if ( auto g = this->global() )
  {
    g->registry.set( "statistics", this->name(), _impl, false);
  }
  
}


namespace 
{
  void wlog_metric( std::stringstream& os, long long int val, int metric)
  {
    val/=metric;
    os << val;
    switch (metric)
    {
      case 1 : os << "ns"; break;
      case 1000 : os << "μs"; break;
      case 1000000 : os << "ms"; break;
      case 1000000000 : os << "s"; break;
      default: os << "?s";
    }
  }
  
  void wlog( std::stringstream& os, std::string name, long long int val, int metric)
  {
    if ( metric < 0 )
      return;

    os << name << ":";
    if ( metric==0 ) 
    {
      if (val!=0) val = 1000000000/val;
      else val = -1;
      os << val << "ps";
    }
    else
      wlog_metric( os, val, metric);
    os << " ";
  }
}

void stat_domain::initialize() 
{
  _wbtp = this->get_target<ibtp>( this->options().btp_target );

  if ( auto wf = this->get_workflow() ) 
  {
    std::weak_ptr<stat_domain::impl> wimpl = _impl;
    auto wbtp = _wbtp;
    auto log = this->options().log;
    int metric = this->options().log_metric;
    _stat_wf_id = wf->create_timer( std::chrono::milliseconds(1000), [this, wimpl, wbtp, log, metric]()
    {
      if ( auto pimpl = wimpl.lock() )
      {
        int count = pimpl->count();
        for ( int i = 0; i < count; ++i)
        {
          std::string name = pimpl->get_name(i);
          while (auto ag = pimpl->pop(i) )
          {
            if ( !log.empty() )
            {
              std::stringstream ss;
              ss << name << " ";
              ss << "count:"    << ag->count << " ";
              wlog( ss, "min", ag->min, metric );
              wlog( ss, "perc80", ag->perc80, metric );
              wlog( ss, "perc99", ag->perc99, metric );
              wlog( ss, "perc100", ag->perc100, metric );
              wlog( ss, "max", ag->max, metric );
              ss << "lossy:"    << ag->lossy << " ";
              WFC_LOG_MESSAGE(log, ss.str() )
            }

            if ( auto pbtp = wbtp.lock() )
            {
              // Структура aggregated в wfc немного отличается от btp 
              auto req = std::make_unique<btp::request::add>();
              req->name = name;
              // сначала переносим data
              req->cl = std::move(ag->data);
              // потом все остальное (req->ag.data не сериализуется! только req->cl )
              req->ag = std::move(*ag);
              req->ts = req->ag.ts;
              this->prepare_(req);
              pbtp->add( std::move(req), nullptr );
            }
          }
        }
      }
      else
        return false;
      return true;
    });
  }
  else
  {
    abort();
  }
}

void stat_domain::stop(const std::string&) 
{
  if ( auto wf = this->get_workflow() ) 
    wf->release_timer(_stat_wf_id);

  if ( auto g = this->global() )
  {
    g->registry.erase( "statistics", this->name());
  }
}

void stat_domain::prepare_(btp::request::add::ptr& /*add*/)
{
  /*
  add->ag.min /= 1000;
  add->ag.perc100 /= 1000; 
  add->ag.perc99 /= 1000;
  add->ag.perc95 /= 1000;
  add->ag.perc80 /= 1000;
  add->ag.perc50 /= 1000;
  add->ag.avg /= 1000;
  add->ag.max /= 1000;
  for (auto& v : add->ag.data)
    v /= 1000;
  */
}
/*
istat::meter_ptr stat_domain::create_meter(const std::string& rate_name, const std::string& size_name)
{
  if ( this->suspended() )
    return nullptr;
  return _impl->create_meter(rate_name, size_name);
}

istat::meter_ptr stat_domain::clone_meter(meter_ptr m, size_t count )
{
  if ( this->suspended() )
    return nullptr;
  return _impl->clone_meter(m, count);
}
*/

/*
int stat_domain::reg_name(const std::string& name) 
{
  return _impl->reg_name(name);
}

stat_domain::meter_ptr stat_domain::create_meter(int id, size_t count) 
{
  if ( this->suspended() )
    return nullptr;

  return _impl->create_meter(id, count);
}

stat_domain::meter_ptr stat_domain::create_meter(const std::string& name, size_t count) 
{
  if ( this->suspended() )
    return nullptr;

  return _impl->create_meter(name, count);
}

stat_domain::meter_ptr stat_domain::clone_meter(meter_ptr m, size_t count) 
{
  if ( this->suspended() )
    return nullptr;
  return _impl->clone_meter(m, count);
}
*/


}
