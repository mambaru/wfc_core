//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2013-2015
//
// Copyright: See COPYING file that comes with this distribution
//

#include "btp_deprecated_gateway_multiton.hpp"
#include "btp_deprecated_gateway.hpp"

#include <wfc/module/multiton.hpp>
#include <wfc/module/instance.hpp>
#include <wfc/name.hpp>

namespace wfc{ namespace core{

namespace {

  WFC_NAME2(component_name, "btp-deprecated-gateway")

  class impl
    : public ::wfc::jsonrpc::gateway_multiton< 
        component_name, 
        gateway::btp_deprecated_method_list, 
        gateway::btp_deprecated_interface
    > 
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

btp_deprecated_gateway_multiton::btp_deprecated_gateway_multiton()
  : wfc::component( std::make_shared<impl>() )
{
}

}}
