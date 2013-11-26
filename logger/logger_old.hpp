#pragma once
#include <vector>
#include <mutex>
#include <comet/logger/ilogger.hpp>
#include <comet/core/imodule.hpp>
#include <comet/inet/imux.hpp>

#include "logger_config.hpp"
#include <unordered_map>

namespace mamba{ namespace comet{

class log_writer;

class logger
  : public ilogger
{
  
public:
  
  
  virtual ~logger();

  logger(std::shared_ptr<global> gl);
  // ilogger

  virtual void initialize(const std::string& name, std::stringstream& str);
  virtual void write(const std::string& name, const std::string& ident,  const std::string& str);

  // logger_module
  void configure(const logger_config& conf);
  
private:
  void _create_log(const std::string& name, const log_writer_config& lwc);
  std::shared_ptr<log_writer> _find(const std::string& name) const;
  
private:
  std::shared_ptr<global> _global;
  logger_config _config;
  std::mutex _mutex;
  std::unordered_map<std::string, std::shared_ptr<log_writer> > _log_map;
  
private:
  std::string _tmp_filename;
};

}}
