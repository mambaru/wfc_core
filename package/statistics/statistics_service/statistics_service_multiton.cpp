//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2013-2015
//
// Copyright: See COPYING file that comes with this distribution
//

#include "statistics_service_multiton.hpp"
#include "statistics_service.hpp"

#include <wfc/module/multiton.hpp>
#include <wfc/name.hpp>

namespace wfc{ namespace core{

namespace 
{
  WFC_NAME2(service_name, "statistics-service")

  class impl
    : public ::wfc::jsonrpc::service_multiton< service_name, statistics::service_method_list> 
  {};
}

statistics_service_multiton::statistics_service_multiton()
  : wfc::component( std::make_shared<impl>() )
{
}

}}
