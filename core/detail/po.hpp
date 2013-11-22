#include <string>

namespace mamba{ namespace comet{ namespace detail{

struct po
{
  bool help;
  bool daemonize;
  bool coredump;
  bool generate;
  bool autoup;
  time_t autoup_timeout;
  std::string module_help;
  std::string instance_name;
  std::string config_path;
  std::string generate_name;

  po()
    : help(false)
    , daemonize(false)
    , coredump(false)
    , generate(false)
    , autoup(false)
    , autoup_timeout(0)
  {}

  static po parse(int argc, char** argv);
};

 

}}}
