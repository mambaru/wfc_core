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
