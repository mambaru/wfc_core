
#include <wfc/iinterface.hpp>
#include "stat_domain.hpp"
#include <wfc/stat/stat.hpp>
//#include <wrtstat/wrtstat.hpp>

namespace wfc{
 

class stat_domain::impl
  : public ::wfc::stat
{
public:
  impl(const stat_config& opt )
    : stat( opt)
  {}
};


stat_domain::~stat_domain()
{
}

void stat_domain::reconfigure()
{
  /*
  auto opt = this->options();
  opt.step_ts = 5000000;*/

  _impl = std::make_shared<impl>( this->options() );
  if ( auto wf = this->get_workflow() ) 
    wf->release_timer(_stat_wf_id);
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
  if ( auto wf = this->get_workflow() ) 
  {
    std::weak_ptr<stat_domain::impl> wimpl = _impl;
    auto log = this->options().log;
    int metric = this->options().log_metric;
    _stat_wf_id = wf->create_timer( std::chrono::milliseconds(1000), [wimpl, log, metric]()
    {
      if ( auto pimpl = wimpl.lock() )
      {
        int count = pimpl->count();
        for ( int i = 0; i < count; ++i)
        {
          while (auto ag = pimpl->pop(i) )
          {
            if ( !log.empty() )
            {
              std::stringstream ss;
              ss << "count: "    << ag->count << " ";
              wlog( ss, "min", ag->min, metric );
              wlog( ss, "perc80", ag->perc80, metric );
              wlog( ss, "perc99", ag->perc99, metric );
              wlog( ss, "perc100", ag->perc100, metric );
              wlog( ss, "max", ag->max, metric );
              WFC_LOG_MESSAGE(log, ss.str() )
            }
            /*
            std::cout << "YES" << std::endl;
            std::cout << "\tcount: " << ag->count << std::endl;
            std::cout << "\tmax: " << ag->max << std::endl;
            std::cout << "\tmin: " << ag->min << std::endl;
            */
          }
          /*else
            std::cout << "no" << std::endl;
          */
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



int stat_domain::reg_name(const std::string& name) 
{
  /*if ( this->suspended() )
    return 0;*/
  return _impl->reg_name(name);
}

stat_domain::meter_ptr stat_domain::create_handler(int id) 
{
  if ( this->suspended() )
    return nullptr;
  return _impl->create_handler(id);
}

stat_domain::meter_ptr stat_domain::create_handler(const std::string& name) 
{
  if ( this->suspended() )
    return nullptr;

  return _impl->create_handler(name);
}




}
