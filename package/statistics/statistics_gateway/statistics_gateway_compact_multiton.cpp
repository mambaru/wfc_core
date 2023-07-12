
//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2013-2015, 2023
//
// Copyright: See COPYING file that comes with this distribution
//

#include "statistics_gateway_compact_multiton.hpp"
#include "statistics_gateway_compact.hpp"

#include <wfc/module/multiton.hpp>
#include <wfc/module/instance.hpp>
#include <wfc/name.hpp>

namespace wfc{ namespace core{

namespace {

  WFC_NAME2(component_name, "statistics-gateway-compact")
  class impl
    : public ::wfc::jsonrpc::gateway_multiton<
                component_name,
                statistics::compact::gateway_method_list,
                statistics::compact::gateway_interface
      >
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

statistics_gateway_compact_multiton::statistics_gateway_compact_multiton()
  : wfc::component( std::make_shared<impl>() )
{
}

}}
