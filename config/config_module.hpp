#pragma once

#include <wfc/core/imodule.hpp>
#include <memory>
#include <string>

#include "config.hpp"

namespace wfc{

struct global;
class config;

class config_module: public imodule
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
  std::shared_ptr<config> _config;
  std::shared_ptr<global> _global;
  config_config _config_config;
};

}
