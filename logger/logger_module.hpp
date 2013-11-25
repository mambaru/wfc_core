#pragma once

#include "logger_config.hpp"
#include <comet/core/imodule.hpp>
#include <memory>
#include <string>


namespace mamba{ namespace comet{

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
  virtual void create( std::weak_ptr<global> g );
  virtual void configure(const std::string& conf);
  virtual void initialize();
  virtual void start();
  virtual void stop();
  virtual void idle();
private:
  std::shared_ptr<logger> _logger;
  std::shared_ptr<global> _global;
  logger_config _logger_config;
};

}}
