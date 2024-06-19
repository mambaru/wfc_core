
#include "prometheus_domain.hpp"

#include <prometheus/gauge.h>
#include <prometheus/counter.h>
#include <prometheus/exposer.h>
#include <prometheus/registry.h>

#include <iostream>

namespace wfc{ namespace core{

using namespace prometheus;

class prometheus_domain::impl
{
public:
  explicit impl(const prometheus_config& opt)
    : _opt(opt)
  {}

  void start(const std::string& name, std::weak_ptr<icore> wcore)
  {
    _wcore = wcore;

    _exposer = std::make_shared<Exposer>(_opt.addr + ":" + _opt.port, 1);

    _registry = std::make_shared<Registry>();

    _exposer->RegisterCollectable(_registry);


    _pcounter = &(BuildGauge()
                    .Name("daemon_status")
                    .Help("The status of the '" + name + "' daemon")
                    .Register(*_registry));


    _perrors = &( _pcounter->Add({{"status","errors"}}) );
    _pwarnings = &( _pcounter->Add({{"status","warnings"}}) );
    _pfatals = &( _pcounter->Add({{"status","fatals"}}) );
    _phang = &( _pcounter->Add({{"status","hangs"}}) );
    _pok = &( _pcounter->Add({{"status","ok"}}) );
    _pstage = &( _pcounter->Add({{"status","stage"}}) );
    _ptime = &( _pcounter->Add({{"status","update_count"}}) );
  }

  void update()
  {
    if (auto pcore = _wcore.lock() )
    {
      _ptime->Increment();
      _perrors->Set(0);
      _pwarnings->Set(0);
      _pfatals->Set(0);
      _phang->Set(0);

      core_stage stage;
      icore::status_list_t sl;
      core_status cs = pcore->get_status(&stage, &sl);
      if ( cs == core_status::OK)
        _pok->Set(1);
      else
        _pok->Set(0);

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
    }
  }

  void stop()
  {
    _exposer->RemoveCollectable(_registry);
    _exposer.reset();
    _registry.reset();
  }
private:
  std::weak_ptr<icore> _wcore;
  prometheus_config _opt;
  std::shared_ptr<Exposer> _exposer;
  std::shared_ptr<Registry> _registry;

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

void prometheus_domain::initialize()
{
  std::string name = this->global()->instance_name;
  _prometheus->start( name, this->get_target<icore>("core"));
  if ( _timer != -1 )
    this->get_workflow()->release_timer(_timer);
  std::weak_ptr<impl> wp = _prometheus;
  time_t update_ms = this->options().update_ms;
  _timer = this->get_workflow()->create_timer(std::chrono::milliseconds(update_ms), [wp]()->bool{
    if (auto p = wp.lock() )
      p->update();
    return true;
  });
}

void prometheus_domain::start()
{
}

void prometheus_domain::restart()
{
}

void prometheus_domain::stop()
{
  _prometheus->stop();
  _prometheus.reset();
}

}}
