#pragma once

#include <comet/core/icore.hpp>
#include "core_config.hpp"
namespace mamba{ namespace comet{

class core
  : public icore
{
public:
  
  virtual ~core();
  
  /// icore
  virtual void reconfigure();
  virtual void run( int argc, char* argv[], std::weak_ptr<global> g );
  virtual void stop( );

  //
  void configure(const core_config& conf);
private:
  std::weak_ptr<global> _global;
  core_config _conf;
};

}}
