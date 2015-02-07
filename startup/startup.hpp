#pragma once
#include <vector>
#include <chrono>
#include <wfc/core/istartup.hpp>
//#include <wfc/module/imodule.hpp>
//#include <wfc/inet/imux.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio.hpp>
#include "startup_config.hpp"

namespace wfc{

//namespace detail{ struct po; }

class idle_timer;
  
class startup_impl
  : public istartup
{
  
public:

  virtual ~startup_impl();
  startup_impl( std::weak_ptr<global> g );
  
  virtual bool startup( int argc, char* argv[] );

  // startup_module
  void configure(const startup_config& conf);
  void reconfigure();
  /*
private:
  typedef std::pair<std::string, std::shared_ptr<imodule> > module_pair;
  typedef std::vector<module_pair> module_vector;
*/
private:
  bool _startup( int argc, char** argv);
  void _generate( const std::string& type, const std::string& path );
  //bool _poccess_po( detail::po* p);
  void _show_usage();
  void _show_help();
  void _show_info();
  void _show_module_info(const std::string& module_name);
  
private:
  std::shared_ptr<global> _global;
  startup_config _conf;
};

}
