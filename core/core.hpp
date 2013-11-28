#pragma once
#include <vector>
#include <chrono>
#include <comet/core/icore.hpp>
#include <comet/core/imodule.hpp>
#include <comet/inet/imux.hpp>

#include "core_config.hpp"
namespace mamba{ namespace comet{

//namespace detail{ struct po; }

class core
  : public icore
{
  
public:

  virtual ~core();
  core();
  
  /// icore
  virtual void reconfigure();
  virtual int run( int argc, char* argv[], std::weak_ptr<global> g );
  virtual void stop( );

  // core_module
  void configure(const core_config& conf);
  
private:
  typedef std::pair<std::string, std::shared_ptr<imodule> > module_pair;
  typedef std::vector<module_pair> module_vector;

  void _prepare(module_vector& mv);
  
  void _sunrise();
  void _configure( const module_vector& m );
  void _initialize(const module_vector& m);
  void _start(const module_vector& m);
  void _stop();
  int _main_loop();

private:
  // return true if ready for running
  bool _startup( int argc, char** argv);
  void _generate( const std::string& type, const std::string& path );
  //bool _poccess_po( detail::po* p);
  void _show_usage();
  void _show_help();
  void _show_info();
  void _show_module_info(const std::string& module_name);
  
private:
  std::shared_ptr< inet::imux<> > _mux;
  std::shared_ptr<global> _global;
  core_config _conf;

  //typedef std::chrono::duration<time_t, std::chrono::milliseconds> duration_type;
  //typedef std::chrono::time_point<std::chrono::steady_clock, std::chrono::milliseconds> time_point_t;
  typedef std::chrono::steady_clock::time_point time_point;
  time_point _idle_time;

  bool _reconfigure_flag;
  bool _stop_flag;
  //std::chrono::milliseconds _idle_time;
  
};

}}
