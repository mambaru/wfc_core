//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2013-2015
//
// Copyright: See COPYING file that comes with this distribution
//

#include "statistics_gateway_multiton.hpp"
#include "statistics_gateway.hpp"

#include <wfc/module/multiton.hpp>
#include <wfc/module/instance.hpp>
#include <wfc/name.hpp>

namespace wfc{ namespace core{

namespace {

  WFC_NAME2(component_name, "statistics-gateway")
  class impl
    : public ::wfc::jsonrpc::gateway_multiton< component_name, statistics::gateway_method_list, statistics::gateway_interface> 
  {
  public:
    virtual std::string interface_name() const override
    {
      return std::string("wfc::statistics::istatistics");
    }

    virtual std::string description() const override
    {
      return "Gateway for BTP system";
    }
  };
}

statistics_gateway_multiton::statistics_gateway_multiton()
  : wfc::component( std::make_shared<impl>() )
{
}

}}
