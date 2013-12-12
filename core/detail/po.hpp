#include <string>

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

 

}}
