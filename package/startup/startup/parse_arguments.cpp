#include "parse_arguments.hpp"
#include <boost/program_options.hpp>
#include <iow/boost.hpp>
#include <iostream>
#include <wjson/wjson.hpp>

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
  pa.program_path =  boost::filesystem::system_complete(argv[0]).parent_path().lexically_normal().native();
  pa.program_name = ::boost::filesystem::path(argv[0]).filename().native();

  pa.usage = ( argc == 1 );

  using namespace boost::program_options;
  typedef std::vector<std::string> vstrings;
  //typedef std::vector<vstrings> vstrings2;
  vstrings generate_options;

  options_description desc("Allowed options");
  options_description desc_startup("Startup options");

  desc.add_options()
    ("help,h", value<vstrings>(&pa.help_options)->multitoken()->zero_tokens(), "Produce help message. Use component-name[:args] as arg for produce component help message.")
    ("version,v", value<bool>(&pa.version)->zero_tokens(), "Display program version information")
    ("info,i", value< vstrings >(&pa.info_options)->multitoken()->zero_tokens(), "Display build information [package-list]")
    ("module-list", value<bool>(&pa.module_list)->zero_tokens(), "Display list of modules from all packages")
    ("component-list", value<bool>(&pa.component_list)->zero_tokens(), "Display all available components")
    ("generate,G", value< vstrings >(&generate_options)->multitoken()->zero_tokens(),
                   "Generate configuration [object-name[:arg]]. Use -C option for write to file.")
    ("check-config,V", value<std::string>(&pa.check_config)->default_value(""), "Load and parse configuration file without start")

    ;

  vstrings object_options;
  vstrings startup_options;
  vstrings ini_list;
  vstrings startup_ini_list;

  std::string autoup_timeout;
  desc_startup.add_options()
    ("user,u", value<std::string>(&pa.user_name)->default_value(""), "Change user name")
    ("working-directory,w", value<std::string>(&pa.working_directory)->default_value(""), "Change working directory")
    ("daemonize,d", value<bool>(&pa.daemonize)->zero_tokens(), "Run as daemon")
    ("wait-daemonize,W", value<bool>(&pa.wait_daemonize)->zero_tokens(), "Do not leave the parent process until the start procedure is complete")

    ("autoup,a", value<std::string>(&autoup_timeout)->default_value(""), "Auto restart daemon [minimum uptime in sec or '1d2h4m5s' format]")
    ("success-autoup,A", value<bool>(&pa.success_autoup)->zero_tokens(), "Auto restart daemon with success")
    ("shutdown-time,T", value<std::string>(&pa.shutdown_time)->default_value(""), "Shutdown time in the format '22:00:00' + working_time")
    ("working-time,t", value<std::string>(&pa.working_time)->default_value(""), "Work time daemon in seconds or '1d2h4m5s' format after shutdown_time")
    ("restart-by-timer,R", value<bool>(&pa.restart_by_timer)->zero_tokens(), "Restart daemon by timer if set (-T or -t) and -a")
    ("coredump,c", value<bool>(&pa.coredump)->implicit_value(true)->default_value(false), "Allow or deny core dump")
    ("name,n", value<std::string>(&pa.instance_name), "Unique daemon instance name")
    ("config,C", value<std::string>(&pa.config_path)->default_value(""), "Path to the configuration file")
    ("pid-dir,P", value<std::string>(&pa.pid_dir), "Directory for pid file")
    ("object-options,O", value< vstrings >(&object_options)->multitoken(), "<<object-name>>:arg=value[:arg2=value2...] custom options for instance objects")
    ("startup-options,S", value< vstrings >(&startup_options)->multitoken(), "<<object-name>>:arg=value[:arg2=value2...] custom option for instance objects only for first start (сleaned after restart by autoup)")
    ("ini", value< vstrings >(&ini_list)->multitoken(), "List of ini files for variable substitution in the"
            " ${INI:<<section.name>>:<<default>>} format configuration file"
            "Variables from this list override variables from the list of ini-files specified in the configuration")
    ("startup-ini", value< vstrings >(&startup_ini_list)->multitoken(),
            "Similar to --ini but only for first start (сleaned after restart by autoup)");

  desc.add(desc_startup);

  variables_map vm;
  parsed_options parsed = command_line_parser(argc, argv).options(desc).run();
  store(parsed, vm);
  notify(vm);

  if ( !pa.working_directory.empty() )
  {
     pa.working_directory =  boost::filesystem::system_complete(pa.working_directory).lexically_normal().native();
  }

  if ( !pa.pid_dir.empty() )
  {
     pa.pid_dir =  boost::filesystem::system_complete(pa.pid_dir).lexically_normal().native();
  }

  if ( !autoup_timeout.empty() )
  {
    if ( autoup_timeout.end() != wjson::parser::parse_integer(autoup_timeout.begin(), autoup_timeout.end(), nullptr ) )
      autoup_timeout = "\"" + autoup_timeout + "\"";
    wjson::time_interval<time_t, 1>::serializer serializer;
    wjson::json_error er;
    serializer(pa.autoup_timeout, autoup_timeout.begin(), autoup_timeout.end(), &er);
    if (er)
    {
      pa.errorstring = "Program option autoup_timeout invalid value";
      return;
    }
  }
  else
    pa.autoup_timeout = -1;


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
      pa.generate_options[ std::string(g.begin(), g.begin() +  static_cast<std::ptrdiff_t>(pos) ) ]
        = std::string(g.begin() + static_cast<std::ptrdiff_t>(pos) + 1 , g.end() );
    }
  }

  pa.object_options = parse_custom_options( object_options );
  pa.startup_options = parse_custom_options( startup_options );
  pa.ini_list = ini_list;
  pa.startup_ini_list = startup_ini_list;

  if ( pa.help )
  {
    std::stringstream ss;
    desc.print(ss);

    if ( !pa.object_options.empty() )
    {
      ss << std::endl << "Instance options:" << std::endl;
      for ( const auto& ins : pa.object_options)
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
    if (  pa.config_path.empty() )
      pa.instance_name = pa.program_name;
    else
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
      key = std::string(opt.begin(), opt.begin() + static_cast<std::ptrdiff_t>(beg));
      val = std::string(opt.begin() + static_cast<std::ptrdiff_t>(beg) + 1, opt.end() );
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
        parse_pair(std::string(opt.begin()+static_cast<std::ptrdiff_t>(beg), opt.begin()+static_cast<std::ptrdiff_t>(end)), res);
        beg = end+1;
      }
      else
      {
        parse_pair(std::string(opt.begin()+static_cast<std::ptrdiff_t>(beg), opt.end()), res);
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
      std::string name(opt.begin(), opt.begin() + static_cast<std::ptrdiff_t>(pos));
      std::string value(opt.begin()+ static_cast<std::ptrdiff_t>(pos) + 1, opt.end());
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
    return res;
  }
}

}}
