#pragma once

#include <ctime>
#include "../core.hpp"
#include "status_log_options.hpp"
#include <wfc/core/cpuset.hpp>
#include <wfc/core/core_status_json.hpp>

namespace wfc{  namespace core{

class core::status_log
{
  struct status_item
  {
    typedef std::chrono::time_point<std::chrono::system_clock> time_point_t;
    core_status status = core_status::OK;
    time_point_t time_point;
    std::string message;
  };
  typedef std::list<status_item> status_list_t;
  typedef std::chrono::milliseconds duration_t;
  typedef std::mutex mutex_type;

public:
  status_log(const status_log_options& opt, cpuset* cs) noexcept
    : _opt(opt)
    , _active_map(cs)
  {

  }

  void set_status(core_status st, const std::string& msg)
  {
    std::lock_guard<mutex_type> lock(_mutex);
    status_item item;
    item.status = st;
    item.message = msg;
    item.time_point = std::chrono::system_clock::now();
    switch (st)
    {
      case core_status::WARINING: _warnings.push_back(item); break;
      case core_status::ERROR: _errors.push_back(item); break;
      case core_status::ABORT: _fatals.push_back(item); break;
      case core_status::OK:
        break;
      case core_status::HANG:
        break;
      default:
        break;
    };
    this->remove_expired_();
  }

  void set_stage(core_stage cs)
  {
    std::lock_guard<mutex_type> lock(_mutex);
    _stage = cs;
  }

  core_status get_status(core_stage* stage, std::vector<std::pair<core_status, std::string>>* status_list)
  {
    core_status status = core_status::OK;

    if ( _opt.hang_timeout_ms > 0 )
    {
      std::vector<cpuset::active_thread> active_list;
      _active_map->get_active(&active_list);
      auto hang_time = std::chrono::system_clock::now() - std::chrono::milliseconds(_opt.hang_timeout_ms);
      for (const auto& itm : active_list )
      {
        if ( itm.last_active < hang_time )
        {
          status = core_status::HANG;
          status_list->push_back({core_status::HANG, itm.name});
          auto hang_timeout = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - itm.last_active);
          status_list->back().second += " timeout " + std::to_string(hang_timeout.count() ) + "ms";
        }
      }
    }

    std::lock_guard<mutex_type> lock(_mutex);
    this->remove_expired_();
    if ( !_fatals.empty() )
    {
      status = core_status::ABORT;
      for (const auto& itm : _fatals )
      {
        status_list->push_back({core_status::ABORT, itm.message});
      }
    }

    if ( !_errors.empty() )
    {
      if ( status == core_status::OK && _errors.size()==_opt.errors_limit)
        status = core_status::ERROR;

      for (const auto& itm : _errors )
      {
        status_list->push_back({core_status::ERROR, itm.message});
      }
    }

    if ( !_warnings.empty() )
    {
      if ( status == core_status::OK && _warnings.size()==_opt.warnings_limit )
        status = core_status::WARINING;

      for (const auto& itm : _warnings )
      {
        status_list->push_back({core_status::WARINING, itm.message});
      }
    }

    *stage = _stage;
    return status;
  }

  template<typename T, typename V>
  std::string to_string(V v)
  {
    std::string str;
    typename T::serializer()(v, std::back_inserter(str) );
    return std::string(str.begin()+1, str.end()-1);
  }

  std::string get_status_text(size_t errlogs, size_t wrnlogs)
  {
    core_stage stage;
    std::vector<std::pair<core_status, std::string>> status_list;
    core_status status = this->get_status(&stage, &status_list);

    std::stringstream ss;
    ss << to_string<core_status_json>(status);
    if ( stage != core_stage::IS_RUN )
    {
      ss << " " << to_string<core_stage_json>(stage);
    }
    ss << std::endl;

    if ( errlogs != 0 )
    {
      for (const auto& s  : status_list)
      {
        if ( s.first == core_status::ERROR ) {
          if ( errlogs == 0 ) continue;
          --errlogs;
        }

        if ( s.first == core_status::WARINING ) {
          if ( wrnlogs == 0 ) continue;
          --wrnlogs;
        }

        ss << s.second;
      }
    }

    return ss.str();
  }

private:

  void add_status_(const status_item& itm, status_list_t* sl, duration_t lifetime, size_t limit)
  {
    sl->push_back(itm);
    while ( sl->size() > limit )
      sl->pop_front();
    auto dietime = std::chrono::system_clock::now() - lifetime;
    while ( !sl->empty() )
    {
      if ( sl->front().time_point < dietime )
        sl->pop_front();
      else
        break;
    }
  }

  void remove_expired_()
  {
    remove_expired_(&_warnings, std::chrono::milliseconds(_opt.warnings_ttl_ms), _opt.warnings_limit);
    remove_expired_(&_errors, std::chrono::milliseconds(_opt.errors_ttl_ms), _opt.errors_limit);
  }

  static void remove_expired_(status_list_t* sl, duration_t lifetime, size_t limit)
  {
    while ( sl->size() > limit )
      sl->pop_front();
    auto dietime = std::chrono::system_clock::now() - lifetime;
    while ( !sl->empty() )
    {
      if ( sl->front().time_point < dietime )
        sl->pop_front();
      else
        break;
    }
  }

private:
  core_stage _stage = core_stage::UNDEFINED;
  status_log_options _opt;
  cpuset* _active_map;
  status_list_t _warnings;
  status_list_t _errors;
  status_list_t _fatals;
  mutex_type _mutex;
};

}}
