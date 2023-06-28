
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
  explicit stat_impl(const ::wfc::statistics::statistics_options& opt )
    : statistics(opt)
  {}
};


statistics_domain::~statistics_domain()
{
}

void statistics_domain::reconfigure_basic()
{
  std::lock_guard<mutex_type> lk(_mutex);
  _stat->enable( !this->suspended()  );
}

void statistics_domain::reconfigure()
{
  auto opt =  this->options();
  _suspend_push = opt.suspend_push;
  std::lock_guard<mutex_type> lk(_mutex);
  _stat = std::make_shared<stat_impl>( opt );
  _stat->enable( !this->suspended()  );
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
  if ( auto g = this->global() )
  {
    g->registry.erase( "statistics", this->name());
  }
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
