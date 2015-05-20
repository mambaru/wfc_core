#pragma once

namespace wfc{

struct writer_config
{
  bool sylog;
  time_t lifetime;
  std::string path;
  std::string stdout;
  writer_config()
    : sylog(false)
    , lifetime(0)
    , stdout("clog")
  {}
};

}
