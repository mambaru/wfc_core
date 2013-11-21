#include "core.hpp"
#include <comet/core/global.hpp>
#include <comet/core/imodule.hpp>
#include <vector>
#include <string>
#include <iostream>

/*
  --help                   produce help message
  --generate-help          produce help message
  -n [ --name ] arg        unique daemon instance name
  -d [ --daemonize ] 
  -c [ --coredump ] 
  -a [ --autoup ] 
  -C [ --config-path ] arg path to the configuration file
  --config-channel arg     configuration channel
  --config-target arg      client addr, or name broker in --config-path
  -G [ --generate ] arg    generate of configuration type: minimal, composite, 
                           comet, broker, channel, comet_broker, 
                           channel_broker, full_example, full_example_ex
*/
namespace mamba{ namespace comet{

namespace detail{
  struct po
  {
    bool help;
    bool module_help;
    bool daemonize;
    bool coredump;
    bool generate;
    bool autoup;
    time_t autoup_timeout;
    std::string help_for_module;
    std::string instance_name;
    std::string config_path;
    std::string generate_name;
    
    po()
      : help(false)
      , module_help(false)
      , daemonize(false)
      , coredump(false)
      , generate(false)
      , autoup(false)
      , autoup_timeout(false)
    {}
  };
}

bool core::_po(int argc, char* argv[])
{
  std::vector<std::string> po( argv,  argv+argc);
  
  detail::po po_info;
  size_t size = po.size();
  for (size_t i=0; i < size; )
  {
    if ( po[i] == "--help" || po[i]=="-h")
    {
      po_info.help = true;
    }
    else if (po[i] == "--name" || po[i]=="-n")
    {
      if (i == size - 1)
        abort();
      ++i;
      if ( po[i][0]=='-')
        abort();
        
      po_info.instance_name = po[i];
    }
    else if (po[i] == "--daemonize" || po[i]=="-d")
    {
      po_info.daemonize = true;      
    }
    else if (po[i] == "--autoup" || po[i]=="-a")
    {
      po_info.autoup = true;      
      if (i != size - 1 && po[i+1][0]!='-')
      {
        ++i;
        po_info.autoup_timeout = atoi( po[i].c_str() );
      }
    }
    else if (po[i] == "--coredump" || po[i]=="-c")
    {
      po_info.coredump = true;      
      
    }
    else if (po[i] == "--config" || po[i]=="-C")
    {
      if (i == size - 1)
        abort();
        
      po_info.config_path = po[++i];
    }
    else if (po[i] == "--generate" || po[i]=="-G")
    {
      po_info.generate = true;      
      if (i != size - 1 && po[i+1][0]!='-')
      {
        ++i;
        po_info.generate_name = po[i];
      }
    }
    ++i;
  }
  return _poccess_po(&po_info);
}

bool core::_poccess_po( detail::po* p)
{
  return false;
}

void core::_show_help()
{
  
}

void core::_show_module_help(const std::string& module_name)
{
 
  if ( auto g = _global.lock())
  {
    if (!module_name.empty())
    {
      if ( auto m = g->modules.find(module_name).lock() )
      {
        std::cout << "----------------------------------------------" << std::endl;
        std::cout << module_name << ":" << std::endl;
        std::cout << m->version() << std::endl;
        std::cout << m->description() << std::endl;
      }
    }
    else
    {
      g->modules.for_each([this](const std::string& name, std::weak_ptr<imodule> mod)
      {
        this->_show_module_help(name);
      });
    }
  }
}



void core::_generate( detail::po* p)
{
  
}

}}
