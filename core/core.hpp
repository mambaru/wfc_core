#pragma once

#include <comet/core/icore.hpp>
#include <comet/inet/imux.hpp>

#include "core_config.hpp"
namespace mamba{ namespace comet{

namespace detail{ struct po; }

class core
  : public icore
{
public:
  
  virtual ~core();
  
  /// icore
  virtual void reconfigure();
  virtual void run( int argc, char* argv[], std::weak_ptr<global> g );
  virtual void stop( );

  // core_module
  void configure(const core_config& conf);
private:
  // return true if ready for running
  bool _po( int argc, char* argv[]);
  void _generate( detail::po* p);
  bool _poccess_po( detail::po* p);
  void _show_help();
  void _show_module_help(const std::string& module_name);
  
private:
  std::shared_ptr< inet::imux<> > _mux;
  std::weak_ptr<global> _global;
  core_config _conf;
};

}}
