#pragma once

#include <wfc/logger/ilogger.hpp>
#include "writer_config.hpp"
#include <mutex>

namespace wfc{

class logger_writer
  : public ilogger
{
public:
  logger_writer();

  void initialize( const writer_config& conf );
  
  virtual void write(const std::string& name, const std::string& ident,  const std::string& str);  
  
private:
  
  std::mutex _mutex;
  std::string _filename;
  writer_config _conf;
};

}
