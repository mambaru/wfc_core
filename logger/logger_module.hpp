#pragma once

#include "logger_module_config.hpp"
#include <wfc/module/imodule.hpp>
#include <memory>
#include <string>


namespace wfc{

struct global;
class logger;

class logger_module: public imodule
{
public:
  /// imodule
  virtual priority startup_priority() const;
  virtual priority shutdown_priority() const;
  virtual std::string version() const;
  virtual std::string description() const;
  virtual std::string generate(const std::string& type) const;
  virtual bool parse_config(const std::string& conf);
  virtual void create( const std::string& name, std::shared_ptr<global> g );
  virtual void configure(const std::string& conf, const std::string&);
  virtual void initialize(const std::string&);
  virtual void start(const std::string&);
  virtual void stop(const std::string&);
  virtual void shutdown(const std::string&);
  virtual void idle();
private:
  void _create_single();
  void _create_multi();
  void _reg_loggers();
  void _unreg_loggers();
private:
  std::shared_ptr<logger> _config_log;
  std::shared_ptr<logger> _daemon_log;
  std::shared_ptr<logger> _common_log;
  std::shared_ptr<logger> _debug_log;
  std::shared_ptr<logger> _trace_log;
  
  std::shared_ptr<global> _global;
  logger_module_config    _config;
};

}
