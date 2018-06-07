#include "parse_arguments.hpp"
#include <boost/program_options.hpp>

#include <iow/boost.hpp>

#include <iostream>
namespace wfc{ namespace core{
  
namespace 
{
  inline void parse_pair( const std::string& opt, program_arguments::map1& res);
  
  inline void parse_options( const std::string& opt, program_arguments::map1& res);

  inline void parse_instance( const std::string& opt, program_arguments::map2& res);
  
  inline program_arguments::map2 parse_custom_options( std::vector<std::string> opts);
}

void parse_arguments(program_arguments& pa, int argc, char* argv[])
try
{
  pa.program_name = ::boost::filesystem::path(argv[0]).filename().native();
  pa.usage = ( argc == 1 );
  if ( pa.usage )
    return;
  
  using namespace boost::program_options;
  typedef std::vector<std::string> vstrings;
  //typedef std::vector<vstrings> vstrings2;
  vstrings generate_options;

  options_description desc("Allowed options");
  options_description desc_startup("Startup options");

  desc.add_options()
    ("help,h", value<bool>(&pa.help)->zero_tokens(), "Produce help message")
    ("version,v", value<bool>(&pa.help)->zero_tokens(), "Display program version information")
    ("info,i", value< vstrings >(&pa.info_options)->multitoken()->zero_tokens(), "Display build information [package-list]")
    ("module-list", value<bool>(&pa.module_list)->zero_tokens(), "Display list of modules from all packages")
    ("component-list", value<bool>(&pa.component_list)->zero_tokens(), "Display all available components")
    ("generate,G", value< vstrings >(&generate_options)->multitoken()->zero_tokens(), "Generate configuration [object-name[:arg]]. Use -C option for write to file.")
    ("check-config", value<std::string>(&pa.check_config)->default_value(""), "Load and parse configuration file without start")
    
    ;

  vstrings instance_options;
  vstrings startup_options;

  desc_startup.add_options()
    ("user,u", value<std::string>(&pa.user_name)->default_value(""), "Change user name")
    ("working-directory,w", value<std::string>(&pa.working_directory)->default_value(""), "Change working directory")
    ("daemonize,d", value<bool>(&pa.daemonize)->zero_tokens(), "Run as daemon")
    ("wait-daemonize,W", value<bool>(&pa.wait_daemonize)->zero_tokens(), "Do not leave the parent process until the start procedure is complete")
    ("autoup,a", value<time_t>(&pa.autoup_timeout)->default_value(-1), "Auto restart daemon [minimum uptime in sec]")
    ("coredump,c", value<bool>(&pa.coredump)->zero_tokens(), "Allow core dump")
    ("success-autoup,A", value<bool>(&pa.success_autoup)->zero_tokens(), "Auto restart daemon with success")
    ("name,n", value<std::string>(&pa.instance_name), "Unique daemon instance name")
    ("config,C", value<std::string>(&pa.config_path)->default_value(""), "Path to the configuration file")
    ("pid-dir,P", value<std::string>(&pa.pid_dir), "Directory for pid file")
    ("instance-options,O", value< vstrings >(&instance_options)->multitoken(), "<<instance-name>>:arg=value[:arg2=value2...] custom options for instance objects")
    ("startup-options,S", value< vstrings >(&startup_options)->multitoken(), "<<instance-name>>:arg=value[:arg2=value2...] custom option for instance objects only for first start (сleaned after autoup)");

  desc.add(desc_startup);

  variables_map vm;
  parsed_options parsed = command_line_parser(argc, argv).options(desc).run();
  store(parsed, vm);
  notify(vm);

  pa.help = vm.count("help");
  pa.version = vm.count("version");
  pa.generate = vm.count("generate");
  pa.info = vm.count("info");
  pa.autoup = ( pa.autoup_timeout!=-1 );

  for ( const auto& g : generate_options )
  {
    size_t pos = g.find(':');
    if ( pos == std::string::npos )
    {
      pa.generate_options[g] = "";
    }
    else
    {
      pa.generate_options[ std::string(g.begin(), g.begin() +  pos) ]
        = std::string(g.begin() + pos + 1 , g.end() );
    }
  }
  
  pa.instance_options = parse_custom_options( instance_options );
  pa.startup_options = parse_custom_options( startup_options );
  
  if ( pa.help )
  {
    std::stringstream ss;
    desc.print(ss);
    
    if ( !pa.instance_options.empty() )
    {
      ss << std::endl << "Instance options:" << std::endl;
      for ( const auto& ins : pa.instance_options)
      {
        ss << "  " << ins.first << ":" << std::endl;
        for ( const auto& val : ins.second)
        {
          ss << "\t" << val.first<<"=" << val.second <<  std::endl;
        }
      }
    }
    pa.helpstring = ss.str();
  }

  if ( pa.instance_name.empty() )
  {
    pa.instance_name = pa.program_name + "-" + ::boost::filesystem::path(pa.config_path).filename().native();
  }
}
catch( const ::boost::program_options::error& e)
{
  pa.errorstring = e.what();
}
catch( const std::exception& e)
{
  pa.errorstring = e.what();
}
catch(...)
{
  pa.errorstring = "unhandled exception: Parameter is not valid ";
}

namespace 
{
  inline void parse_pair( const std::string& opt, program_arguments::map1& res)
  {
    size_t beg = opt.find('=');
    std::string key = opt;
    std::string val ;
    if ( beg != std::string::npos )
    {
      key = std::string(opt.begin(), opt.begin() + beg);
      val = std::string(opt.begin() + beg + 1, opt.end() );
    }
    res[key]=val;
  }
  
  inline void parse_options( const std::string& opt, program_arguments::map1& res)
  {
    size_t beg = 0;
    while ( beg != std::string::npos )
    {
      size_t end = opt.find(":", beg);
      if ( end != std::string::npos)
      {
        parse_pair(std::string(opt.begin()+beg, opt.begin()+end), res);
        beg = end+1;
      }
      else
      {
        parse_pair(std::string(opt.begin()+beg, opt.end()), res);
        beg=end;
      }
    }
  }

  inline void parse_instance( const std::string& opt, program_arguments::map2& res)
  {
    if (opt.empty()) return;
    size_t pos = opt.find(":");
    if ( pos == std::string::npos )
    {
      res[opt];
    }
    else
    {
      std::string name(opt.begin(), opt.begin() + pos);
      std::string value(opt.begin()+ pos + 1, opt.end());
      parse_options(value, res[name]);
    }
  }
  
  inline program_arguments::map2 parse_custom_options( std::vector<std::string> opts)
  {
    program_arguments::map2 res;
    for ( const auto& opt: opts)
    {
      parse_instance(opt, res);
    }
    return std::move(res);
  }
}

}}
