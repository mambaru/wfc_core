
#include "statistics_module.hpp"
#include "statistics_aggregator/statistics_multiton.hpp"
#include "system_statistics/system_statistics_multiton.hpp"
#include "gateway/btp_gateway_multiton.hpp"

#include <wfc/module/component_list.hpp>
#include <wfc/name.hpp>

namespace wfc{ namespace core{
  
namespace
{
  WFC_NAME2(module_name, "statistics")

  class impl: public ::wfc::component_list<
    module_name,
    statistics_multiton,
    system_statistics_multiton,
    btp_gateway_multiton
  >
  {};
}

statistics_module::statistics_module()
  : module( std::make_shared<impl>() )
{
}

}}
