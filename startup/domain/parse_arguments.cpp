#include "parse_arguments.hpp"
#include <boost/program_options.hpp>




#include <iostream>
namespace wfc
{

void parse_arguments(program_arguments& pa, int argc, char* argv[])
{
  pa.program_name = argv[0];
  pa.usage = ( argc == 1 );
  if ( pa.usage )
    return;

  
  using namespace boost::program_options;
  typedef std::vector<std::string> vstrings;
  vstrings generate_options;

  options_description desc("Allowed options");
  options_description desc_startup("Startup options");

  // TODO: ->multitoken()
  desc.add_options()
    ("help,h", value<bool>(&pa.help)->zero_tokens(), "produce help message")
    ("info,i", value< vstrings >(&pa.info_options)->multitoken()->zero_tokens(), "show build info [package-list]")
    ("generate,G", value< vstrings >(&generate_options)->multitoken()->zero_tokens(), "generate configuration [object-name [arg]]. Use -C option for write to file.")
    ;

  desc_startup.add_options()
    ("daemonize,d", value<bool>(&pa.daemonize)->zero_tokens(), "run as daemon")
    ("coredump,c", value<bool>(&pa.coredump)->zero_tokens(), "allow core dump")
    ("autoup,a", value<time_t>(&pa.autoup_timeout)->default_value(0), "auto restart daemon [timeout in sec]")
    ("name,n", value<std::string>(&pa.instance_name), "unique daemon instance name")
    ("config,C", value<std::string>(&pa.config_path), "path to the configuration file")
    ("<<instance>>-<<key>> [arg]", "custom option for instance object");

  desc.add(desc_startup);

  variables_map vm;
  parsed_options parsed = command_line_parser(argc, argv).options(desc).run();
  store(parsed, vm);
  notify(vm);
  
//  pa.help = vm.count("help");
  pa.generate = vm.count("generate");
  pa.info = vm.count("info");

  for ( const auto& g : generate_options )
  {
    size_t pos = g.find(':');
    if ( pos == std::string::npos )
    {
      pa.generate_options[g] = "";
    }
  }
}

}
