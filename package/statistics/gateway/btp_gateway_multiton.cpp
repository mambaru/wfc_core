//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2013-2015
//
// Copyright: See COPYING file that comes with this distribution
//

#include "btp_gateway_multiton.hpp"
#include "btp_gateway.hpp"

#include <wfc/module/multiton.hpp>
#include <wfc/module/instance.hpp>
#include <wfc/name.hpp>

namespace wfc{ namespace core{

namespace {

  WFC_NAME2(component_name, "btp-gateway")
  class impl
    : public ::wfc::jsonrpc::gateway_multiton< component_name, gateway::btp_method_list, gateway::btp_interface> 
  {
  public:
    virtual std::string interface_name() const override
    {
      return std::string("wfc::btp::ibtp");
    }

    virtual std::string description() const override
    {
      return "Gateway for BTP system";
    }
  };
}

btp_gateway_multiton::btp_gateway_multiton()
  : wfc::component( std::make_shared<impl>() )
{
}

}}
