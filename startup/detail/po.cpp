#include "po.hpp"
#include <vector>
#include <stdexcept>
#include <cstring>
#include <iostream>


namespace wfc{ namespace detail{

po po::parse(int argc, char** argv)
{
  std::vector<std::string> vpo( argv,  argv+argc);

  if ( vpo.empty() )
    return po();
    
  po po_info;
  size_t size = vpo.size();
  po_info.program_name = vpo[0];
  po_info.usage = ( size == 1 );
  
  for (size_t i=1; i < size; )
  {
    if ( vpo[i] == "--help" || vpo[i]=="-h")
    {
      po_info.help = true;
    }
    else if ( vpo[i] == "--info" || vpo[i]=="-i")
    {
      po_info.info = true;
      if (i != size - 1 && vpo[i+1][0]!='-')
      {
        ++i;
        po_info.info_name = vpo[i];
      }
    }
    else if (vpo[i] == "--name" || vpo[i]=="-n")
    {
      if (i == size - 1 || vpo[i+1][0]=='-')
        throw std::runtime_error("required parameter is missing in 'name'");
      ++i;
      po_info.instance_name = vpo[i];
    }
    else if (vpo[i] == "--daemonize" || vpo[i]=="-d")
    {
      po_info.daemonize = true;
    }
    else if (vpo[i] == "--autoup" || vpo[i]=="-a")
    {
      po_info.autoup = true;
      if (i != size - 1 && vpo[i+1][0]!='-')
      {
        ++i;
        po_info.autoup_timeout = atoi( vpo[i].c_str() );
      }
    }
    else if (vpo[i] == "--coredump" || vpo[i]=="-c")
    {
      po_info.coredump = true;

    }
    else if (vpo[i] == "--config" || vpo[i]=="-C")
    {
      if (i == size - 1)
        abort();

      po_info.config_path = vpo[++i];
    }
    else if (vpo[i] == "--generate" || vpo[i]=="-G")
    {
      po_info.generate = true;
      if ( (i != size - 1) && (vpo[i+1][0]!='-'))
      {
        ++i;
        po_info.generate_name = vpo[i];
      }
    }
    else
    {
      bool is_module_opt = false;
      if ( vpo[i].find("--")!=std::string::npos )
      {
        std::string opt = vpo[i].substr(2);
        size_t pos = opt.find_first_of('-');
        if ( pos != std::string::npos )
        {
          size_t pos2 = opt.find_first_of('=', pos);
          std::string name = opt.substr(0, pos);
          std::string key = opt.substr(pos+1, pos2-pos-1);
          std::string value;
          if (pos2 != std::string::npos)
          {
            value = opt.substr(pos2+1);
          }
          if (!name.empty() && !key.empty())
          {
            po_info.module_options[name][key] = value;
            is_module_opt = true;
          }
        }
      }
      
      if ( !is_module_opt )
        throw std::runtime_error( std::string("unrecognized option '") + vpo[i]+ "'");
    }
    ++i;
  }
  return po_info;
}
}}

#include <boost/program_options.hpp>

namespace wfc{ namespace detail{
  

po2 po2::parse(int argc, char** argv)
{
  using namespace boost::program_options;
  po2 po_info;

  options_description desc("Allowed options");
  desc.add_options()
    ("help", value<bool>(&po_info.help)->zero_tokens(), "produce help message")
    ("info,i", value< std::vector<std::string> >(&po_info.packages)->default_value( std::vector<std::string>(), "" ), "show build info [package-list]")
    ("generate,G", value< std::vector<std::string> >(&po_info.genparams)->default_value( std::vector<std::string>(), "" ), "generate configuration [object-name [arg]]. Use -C option for write to file.");

  options_description desc1("Startup options");
  desc1.add_options()
    ("daemonize,d", value<bool>(&po_info.daemonize)->zero_tokens(), "run as daemon")
    ("coredump,c", value<bool>(&po_info.coredump)->zero_tokens(), "allow core dump")
    ("autoup,a", value<time_t>(&po_info.autoup_timeout)->default_value(0), "auto restart daemon [timeout in sec]")
    ("name,n", value<std::string>(&po_info.instance_name), "unique daemon instance name")
    ("config,C", value<std::string>(&po_info.config_path), "path to the configuration file")
    ("<<instance>>-<<key>> [arg]", "custom option for instance object");

  desc.add(desc1);
  variables_map vm;
  parsed_options parsed = command_line_parser(argc, argv).options(desc).allow_unregistered().run();
  std::vector<std::string> to_pass_further = collect_unrecognized(parsed.options, exclude_positional);
  if ( !to_pass_further.empty() )
  {
    for ( auto s: to_pass_further )
    {
      std::cout << s << std::endl;
    }
    //basic_command_line_parser<char> basic(to_pass_further);
   
    for (const auto& o : parsed.options) 
    {
      if ( !o.unregistered )
        continue;
      if (vm.find(o.string_key) == vm.end()) 
      {
        // an unknown option
        std::cout << "------> " << o.string_key << "-=-" ;
        std::cout << o.value[0];//std::string(o.value.begin(), o.value.end() );
        std::cout << std::endl;
      }
    }
  }

  store(parsed, vm);
  notify(vm);
  po_info.info = vm.count("info");
  po_info.generate = vm.count("generate");
  po_info.autoup = vm.count("autoup");
  if ( po_info.help )
  {
    std::stringstream ss;
    ss << desc;
    po_info.helpstring = ss.str();
  }
  return po_info;
}

}}
