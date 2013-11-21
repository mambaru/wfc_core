#include "core.hpp"
#include <comet/inet/epoller.hpp>
#include <comet/core/global.hpp>

namespace mamba{ namespace comet{

core::~core()
{
  
}

void core::reconfigure()
{
  
}

void core::run( int argc, char* argv[], std::weak_ptr<global> gl )
{
  _global = gl;
  if (auto g = _global.lock() )
  {
    _mux = std::make_shared<inet::epoller>();
    g->mux = _mux;
  }
}

void core::stop( )
{
  
}

void core::configure(const core_config& conf)
{
  _conf = conf;
}

}}
