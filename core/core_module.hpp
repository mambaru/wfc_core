#pragma once

#include <wfc/module/imodule.hpp>
#include <memory>
#include <string>

#include "core_config.hpp"

namespace wfc{

struct global;
class core;

class core_module: public imodule
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
  virtual void configure(const std::string& conf);
  virtual void initialize();
  virtual void start();
  virtual void stop();
  virtual void shutdown();
  virtual void idle();
private:
  std::shared_ptr<core> _core;
  std::weak_ptr<global> _global;
  core_config _config;
};

}
