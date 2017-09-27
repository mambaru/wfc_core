
#include <wfc/iinterface.hpp>
#include "statlog_domain.hpp"

namespace wfc{ namespace core{
 

namespace 
{
  void wlog_metric( std::stringstream& os, long long int val, int metric)
  {
    val*=1000;
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


void statlog_domain::add( request::add::ptr req, response::add::handler cb )
{
  if ( this->bad_request<response::add>(req, cb) )
    return;
  
  auto opt = this->options();
  auto log = opt.log_name;
  int metric = opt.log_metric;
  if ( !log.empty() )
  {
    std::stringstream ss;
    ss << req->name << " ";
    ss << "count:"    << req->count << " ";
    wlog( ss, "min", req->min, metric );
    wlog( ss, "perc80", req->perc80, metric );
    wlog( ss, "perc99", req->perc99, metric );
    wlog( ss, "perc100", req->perc100, metric );
    wlog( ss, "max", req->max, metric );
    ss << "lossy:"    << req->lossy << " ";
    WFC_LOG_MESSAGE(log, ss.str() )
  }
  
  if ( auto res = this->create_response(cb) )
  {
    res->result = true;
    this->send_response(res, cb);
  }
}


}}
