#pragma once
#include <vector>
#include <mutex>

#include <comet/logger/ilogger.hpp>
#include <comet/core/imodule.hpp>
#include <comet/core/global.hpp>
#include <comet/callback/callback_owner.hpp>
#include <comet/inet/imux.hpp>

#include "logger_config.hpp"
#include <unordered_map>

namespace mamba{ namespace comet{

// class log_writer;

class logger
  : public ilogger
  , public callback_owner
{
public:
  logger(const logger_config& conf);
  virtual void initialize(const std::string& name, std::stringstream& str);
  virtual void write(const std::string& name, const std::string& ident,  const std::string& str);  
private:

private:
  std::mutex _mutex;
  std::string _filename;
  logger_config _conf;
};

}}
