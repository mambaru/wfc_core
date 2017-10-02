
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

void statlog_domain::initialize()
{
  _target = this->get_target<istatistics>( this->options().target );
}

void statlog_domain::push( request::push::ptr req, response::push::handler cb )
{
  if ( this->suspended() )
  {
    if ( auto t = _target.lock() )
    {
      t->push( std::move(req), cb);
    }
    return;
  }
  
  if ( this->bad_request<response::push>(req, cb) )
    return;
  
  auto opt = this->options();
  int metric = opt.log_metric;
  auto log = opt.common_log;
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
  
  int id = 0;
  log = opt.legend_log;
  if ( !log.empty() )
  {
    std::lock_guard<mutex_type> lk(_mutex);
    auto itr = _legend.find(req->name);
    if ( itr == _legend.end() )
    {
      id = ++_id_counter;
      _legend[req->name] = id;
      WFC_LOG_MESSAGE(log, id << ":" << req->name );
    }
    else
      id = itr->second;
  }

  log = opt.table_log;
  if ( !log.empty() )
  {
    std::stringstream ss;
    ss << "|" << id 
       << "|" << req->count
       << "|" << req->lossy
       << "|" << req->min
       // << "|" << req->perc0 TODO:
       << "|" << req->perc50
       << "|" << req->perc80
       << "|" << req->perc95
       << "|" << req->perc99
       << "|" << req->perc100
       << "|" << req->max
       << "|" << req->avg
       << "|" << req->ts
       << "|";
    WFC_LOG_MESSAGE(log, ss.str() )

  }

  if ( auto t = _target.lock() )
  {
    t->push( std::move(req), cb);
  }
  else if ( auto res = this->create_response(cb) )
  {
    res->result = true;
    this->send_response(res, cb);
  }
}


}}
