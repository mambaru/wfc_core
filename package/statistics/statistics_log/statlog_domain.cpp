#include <algorithm>
#include <wfc/iinterface.hpp>
#include <wfc/statistics/meters.hpp>
#include "statlog_domain.hpp"
#include <algorithm>

namespace wfc{ namespace core{

namespace
{
  void write_metric( std::stringstream& os, long int val, int metric)
  {
    // statistics_duration::period::den - microseconds
    val*= std::chrono::nanoseconds::period::den/statistics_duration::period::den;
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

  void write_log( std::stringstream& os, const std::string& name, long int val, int metric)
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
      write_metric( os, val, metric);
    os << " ";
  }
}

template<typename T>
void statlog_domain::write_field_(std::stringstream& ss, const std::string& logname, const T& field) const
{
  const auto& sol = _table_format.sequence_of_list;
  bool has_field = std::any_of(
    std::begin(sol), std::end(sol),
    [&logname](const std::string& n) -> bool { return n == logname;}
  );

  if ( !has_field )
  {
    ss << std::setfill(' ') << std::setw(_table_format.default_field_width) << "#" << " |";
    return;
  }

  int field_width = _table_format.default_field_width;
  if ( _table_format.width_map.count(logname) != 0 )
    field_width = _table_format.width_map.at(logname);

  ss << std::setfill(' ') << std::setw(field_width) << field << " |";
}


void statlog_domain::configure()
{
  std::lock_guard<mutex_type> lk(_mutex);
  this->configure_();
}

void statlog_domain::reconfigure()
{
  std::lock_guard<mutex_type> lk(_mutex);
  this->configure_();
}

void statlog_domain::initialize()
{
  _target = this->get_target<istatistics>( this->options().target );
}

void statlog_domain::multi_push( multi_push_ptr req, multi_push_handler cb ) 
{
  if ( this->suspended() )
  {
    if ( auto t = _target.lock() )
    {
      t->multi_push( std::move(req), cb);
    }
    return;
  }
  
  if ( this->bad_request(req, cb) )
    return;

  for (const push_ptr::element_type& p : req->data )
  {
    this->push_log_(p);
  }
  
  if ( auto t = _target.lock() )
  {
    t->multi_push( std::move(req), cb);
  }
  else if ( auto res = this->create_response(cb) )
  {
    res->status = true;
    this->send_response( std::move(res), cb);
  }
}

void statlog_domain::push( push_ptr req, push_handler cb )
{
  if ( this->suspended() )
  {
    if ( auto t = _target.lock() )
    {
      t->push( std::move(req), cb);
    }
    return;
  }

  if ( this->bad_request(req, cb) )
    return;

  this->push_log_(*req);

  if ( auto t = _target.lock() )
  {
    t->push( std::move(req), cb);
  }
  else if ( auto res = this->create_response(cb) )
  {
    res->status = true;
    this->send_response( std::move(res), cb);
  }
}

void statlog_domain::del( del_ptr, del_handler cb )
{
  if ( cb!=nullptr )
    cb(nullptr);
}

void statlog_domain::configure_()
{
  _table_format = this->options().table_format;

    table_format_options::sequence_of_list_t orig_sequence;
    std::swap(orig_sequence, _table_format.sequence_of_list);
    std::vector<std::string> def_sequence = {
        "name", "count", "lossy", "min", "perc50", "perc80",
        "perc95", "perc99", "perc100", "max", "avg", "ts"
    };

    size_t orig_size = orig_sequence.size();
    if ( orig_size > def_sequence.size() )
       orig_size = def_sequence.size(); // Защита от дурака

    for (size_t i = 0 ; i != orig_size; ++i)
    {
      size_t j = 0;
      for (; j < def_sequence.size() && def_sequence[j]!=orig_sequence[i]; ++j);

      if ( orig_sequence[i] == "#" )
      {
        // принудительно отключаем
        _table_format.width_map[def_sequence[j]] = -1;
        if ( i == orig_size - 1 )
        {
          def_sequence.clear();
          // отключаем хвост
          // Если нужно отключить последний элемент в списке, но остальные добавить, то комбинация "#", "$"
        }
      }
      else if ( orig_sequence[i] == "$" )
      {
        _table_format.sequence_of_list.push_back(def_sequence[j]);
      }
      else
      {
        _table_format.sequence_of_list.push_back(orig_sequence[i]);
      }

      def_sequence[j]="#";
    }

    for (const auto& def_intem: def_sequence)
    {
      if ( def_intem == "#" )
        continue;
      _table_format.sequence_of_list.push_back(def_intem);
    }
}

void statlog_domain::push_log_( const push_ptr::element_type& ag)
{
  auto opt = this->options();
  int metric = opt.log_metric;
  auto log = opt.common_log;
  if ( !log.empty() )
  {
    std::stringstream ss;
    ss << ag.name << " ";
    ss << "count:"    << ag.count << " ";
    write_log( ss, "min", ag.min, metric );
    write_log( ss, "perc80", ag.perc80, metric );
    write_log( ss, "perc99", ag.perc99, metric );
    write_log( ss, "perc100", ag.perc100, metric );
    write_log( ss, "max", ag.max, metric );
    ss << "lossy:"    << ag.lossy << " ";
    WFC_LOG_MESSAGE(log, ss.str() )
  }
  
  log = opt.table_format.table_log;
  bool show_legend = opt.table_format.show_legend;
  if ( show_legend )
  {
    std::lock_guard<mutex_type> lk(_mutex);
    auto itr = _legend.find(ag.name);
    if ( itr == _legend.end() )
    {
      _legend.insert(ag.name);
      std::stringstream ss;
      for ( const std::string& field : _table_format.sequence_of_list)
      {
        this->write_field_( ss, field, field );
      }
      
      WFC_LOG_MESSAGE( log, "|" << ss.str() )
    }
  }
  
  if ( !log.empty() )
  {
    std::stringstream ss;
    
    for ( const auto& field : _table_format.sequence_of_list)
    {
      if ( field == "name" ) this->write_field_( ss, "name", ag.name );
      if ( field == "count" ) this->write_field_( ss, "count", ag.count );
      if ( field == "lossy" ) this->write_field_( ss, "lossy", ag.lossy );
      if ( field == "min" )    this->write_field_( ss, "min",   ag.min );
      if ( field == "perc50" ) this->write_field_( ss, "perc50", ag.perc50 );
      if ( field == "perc80" ) this->write_field_( ss, "perc80", ag.perc80 );
      if ( field == "perc95" ) this->write_field_( ss, "perc95", ag.perc95 );
      if ( field == "perc99" ) this->write_field_( ss, "perc99", ag.perc99 );
      if ( field == "perc100" ) this->write_field_( ss, "perc100", ag.perc100 );
      if ( field == "max" ) this->write_field_( ss, "max", ag.max );
      if ( field == "avg" ) this->write_field_( ss, "avg", ag.avg );
      if ( field == "ts" ) this->write_field_( ss, "ts", ag.ts );
      
    }
    
    WFC_LOG_MESSAGE(log, ss.str() )
  }
  
}



}}
