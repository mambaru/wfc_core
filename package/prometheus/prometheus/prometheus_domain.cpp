#include "prometheus_domain.hpp"

#include <prometheus/gauge.h>
#include <prometheus/counter.h>
#include <prometheus/exposer.h>
#include <prometheus/registry.h>

#include <wfc/logger.hpp>
#include <wfc/statistics/statistics.hpp>
#include <chrono>

namespace wfc{ namespace core{

using namespace prometheus;

class prometheus_domain::impl
{
public:
  explicit impl(const prometheus_config& opt)
    : _opt(opt)
    , _last_ok(std::chrono::steady_clock::now())
  {}

  bool ready() const
  {
    return _exposer != nullptr;
  }

  /// Создаёт Exposer/Registry. При ошибке bind — false, процесс не падает.
  bool try_start(const std::string& name, std::weak_ptr<icore> wcore)
  {
    if ( _exposer != nullptr )
      return true;

    _wcore = wcore;
    _name = name;
    const std::string bind = _opt.addr + ":" + _opt.port;

    try
    {
      auto exposer = std::make_shared<Exposer>(bind, 1);
      auto registry = std::make_shared<Registry>();
      exposer->RegisterCollectable(registry);

      _pcounter = &(BuildGauge()
                      .Name("daemon_status")
                      .Help("The status of the '" + name + "' daemon")
                      .Register(*registry));

      _perrors = &( _pcounter->Add({{"status","errors"}}) );
      _pwarnings = &( _pcounter->Add({{"status","warnings"}}) );
      _pfatals = &( _pcounter->Add({{"status","fatals"}}) );
      _phang = &( _pcounter->Add({{"status","hangs"}}) );
      _pok = &( _pcounter->Add({{"status","ok"}}) );
      _pstage = &( _pcounter->Add({{"status","stage"}}) );
      _ptime = &( _pcounter->Add({{"status","update_count"}}) );

      _last_ok = std::chrono::steady_clock::now();
      _pok->Set(1);

      _registry = std::move(registry);
      _exposer = std::move(exposer);
      return true;
    }
    catch (const std::exception& e)
    {
      DOMAIN_LOG_ERROR("prometheus Exposer bind '" << bind << "' failed: " << e.what())
      return false;
    }
    catch (...)
    {
      DOMAIN_LOG_ERROR("prometheus Exposer bind '" << bind << "' failed: unknown exception")
      return false;
    }
  }

  /// Читает core status, обновляет Prometheus gauges (если Exposer жив), возвращает счётчики для BTP.
  status_counts update()
  {
    status_counts counts;
    auto pcore = _wcore.lock();
    if ( !pcore )
      return counts;

    core_stage stage;
    icore::status_list_t sl;
    core_status cs = pcore->get_status(&stage, &sl);

    for (const auto& i: sl)
    {
      switch( i.first )
      {
        case core_status::WARINING: ++counts.warnings; break;
        case core_status::ERROR:    ++counts.errors; break;
        case core_status::HANG:     ++counts.hangs; break;
        case core_status::ABORT:
        case core_status::OK:
        default: break;
      }
    }

    if ( _exposer == nullptr || _ptime == nullptr )
      return counts;

    _ptime->Increment();
    _perrors->Set(0);
    _pwarnings->Set(0);
    _pfatals->Set(0);
    _phang->Set(0);

    const auto now = std::chrono::steady_clock::now();
    if ( cs == core_status::OK )
    {
      _last_ok = now;
      _pok->Set(1);
    }
    else if ( _opt.ok_timeout_ms <= 0 )
    {
      _pok->Set(0);
    }
    else
    {
      const auto bad_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now - _last_ok).count();
      _pok->Set( bad_ms >= _opt.ok_timeout_ms ? 0 : 1 );
    }

    switch (stage)
    {
      case core_stage::IS_RUN:  _pstage->Set(0); break;
      case core_stage::CONFIGURING:  _pstage->Set(3); break;
      case core_stage::INITIALIZING: _pstage->Set(2); break;
      case core_stage::STARTING: _pstage->Set(1); break;
      case core_stage::STOPPING: _pstage->Set(-1); break;
      case core_stage::UNDEFINED: _pstage->Set(-10); break;
      default: break;
    };

    for (const auto& i: sl)
    {
      switch( i.first )
      {
        case core_status::WARINING: _pwarnings->Increment(); break;
        case core_status::ERROR: _perrors->Increment(); break;
        case core_status::ABORT: _pfatals->Increment(); break;
        case core_status::HANG: _phang->Increment(); break;
        case core_status::OK: break;
        default: break;
      }
    }
    return counts;
  }

  void stop()
  {
    if ( _exposer != nullptr && _registry != nullptr )
      _exposer->RemoveCollectable(_registry);
    _exposer.reset();
    _registry.reset();
    _pcounter = nullptr;
    _pok = nullptr;
    _pstage = nullptr;
    _perrors = nullptr;
    _pwarnings = nullptr;
    _pfatals = nullptr;
    _phang = nullptr;
    _ptime = nullptr;
  }
private:
  std::weak_ptr<icore> _wcore;
  prometheus_config _opt;
  std::string _name;
  std::shared_ptr<Exposer> _exposer;
  std::shared_ptr<Registry> _registry;
  std::chrono::steady_clock::time_point _last_ok;

  typedef Family<Gauge> family_counter_t;
  family_counter_t* _pcounter = nullptr;

  Gauge* _pok = nullptr;
  Gauge* _pstage = nullptr;
  Gauge* _perrors = nullptr;
  Gauge* _pwarnings = nullptr;
  Gauge* _pfatals = nullptr;
  Gauge* _phang = nullptr;
  Gauge* _ptime = nullptr;
};


prometheus_domain::~prometheus_domain()
{
}

void prometheus_domain::configure()
{
  auto opt = this->options();
  _prometheus = std::make_shared<impl>( opt );
}

void prometheus_domain::reconfigure()
{
  auto opt = this->options();
  _prometheus = std::make_shared<impl>( opt );
}

void prometheus_domain::init_meters_()
{
  _m_error = value_meter();
  _m_warning = value_meter();
  _m_hang = value_meter();
  if ( auto st = this->get_statistics() )
  {
    _m_error = st->create_value_meter("ERROR");
    _m_warning = st->create_value_meter("WARINING");
    _m_hang = st->create_value_meter("HANG");
  }
}

void prometheus_domain::fire_btp_(const status_counts& counts)
{
  auto fire = [&](value_meter& m, size_t count)
  {
    if ( count == 0 )
      return;
    m.create(static_cast<wrtstat::value_type>(count), count);
  };

  fire(_m_error, counts.errors);
  fire(_m_warning, counts.warnings);
  fire(_m_hang, counts.hangs);
}

void prometheus_domain::update_()
{
  if ( _prometheus == nullptr )
    return;
  auto counts = _prometheus->update();
  this->fire_btp_(counts);
}

void prometheus_domain::initialize()
{
  this->init_meters_();
  std::string name = this->global()->instance_name;
  _prometheus->try_start( name, this->get_target<icore>("core"));
  if ( _timer != -1 )
    this->get_workflow()->release_timer(_timer);
  std::weak_ptr<prometheus_domain> wthis = this->shared_from_this();
  time_t update_ms = this->options().update_ms;
  _timer = this->get_workflow()->create_timer(std::chrono::milliseconds(update_ms), [wthis]()->bool{
    if (auto p = wthis.lock() )
      p->update_();
    return true;
  });
}

void prometheus_domain::start()
{
  this->init_meters_();
  std::weak_ptr<prometheus_domain> wthis = this->shared_from_this();
  time_t update_ms = this->options().update_ms;
  if ( update_ms <= 0 )
    update_ms = 1000;
  this->idle(std::chrono::milliseconds(update_ms), [wthis]() -> bool
  {
    if ( auto pthis = wthis.lock() )
    {
      if ( pthis->_prometheus != nullptr && !pthis->_prometheus->ready() )
      {
        const std::string name = pthis->global()->instance_name;
        if ( pthis->_prometheus->try_start(name, pthis->get_target<icore>("core")) )
        {
          DOMAIN_LOG_MESSAGE("prometheus Exposer started after retry on "
            << pthis->options().addr << ":" << pthis->options().port)
        }
        // ошибка bind уже в DOMAIN_LOG_ERROR внутри try_start
      }
      return true;
    }
    return false;
  });
}

void prometheus_domain::restart()
{
}

void prometheus_domain::stop()
{
  if ( _timer != -1 )
  {
    this->get_workflow()->release_timer(_timer);
    _timer = -1;
  }
  if ( _prometheus != nullptr )
    _prometheus->stop();
  _prometheus.reset();
}

}}
