
/*
#include <string>
#include <map>

#include <wfc/core/global.hpp>

namespace wfc{ namespace detail{

struct po
{
  bool usage;
  bool help;
  bool info;
  bool daemonize;
  bool coredump;
  bool generate;
  bool autoup;
  time_t autoup_timeout;
  std::string program_name;
  std::string info_name;
  std::string instance_name;
  std::string config_path;
  std::string generate_name;

  ::wfc::module_options::module_options_map module_options;

  po()
    : usage(false)
    , help(false)
    , info(false)
    , daemonize(false)
    , coredump(false)
    , generate(false)
    , autoup(false)
    , autoup_timeout(0)
  {}

  static po parse(int argc, char** argv);
};

struct po2
{
  bool help = false;
  bool usage = false;
  bool generate = false;
  bool info = false;
  
  bool daemonize = false;
  bool coredump = false;
  bool autoup = false;
  time_t autoup_timeout = 0;
  
  std::string program_name;
  std::string instance_name;
  std::string config_path;

  std::vector<std::string> genparams;
  std::vector<std::string> packages;
  
  std::string helpstring;
  ::wfc::module_options::module_options_map module_options;

  static po2 parse(int argc, char** argv);
};

}}
*/