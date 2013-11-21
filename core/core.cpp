#include "core.hpp"

namespace mamba{ namespace comet{

core::~core()
{
  
}

void core::reconfigure()
{
  
}

void core::run( int argc, char* argv[], std::weak_ptr<global> g )
{
  _global = g;
}

void core::stop( )
{
  
}

void core::configure(const core_config& conf)
{
  _conf = conf;
}

}}
