#pragma once

#include "core_config.hpp"
#include <wfc/core/imodule.hpp>
#include <string>

namespace wfc{

struct global;
class core;

class core_module
  : public imodule
{
public:
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
  std::shared_ptr<core> _core;
  core_config _config;
};

}
