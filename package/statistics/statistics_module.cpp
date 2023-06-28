
#include "statistics_module.hpp"
#include "statistics/statistics_multiton.hpp"
#include "statistics_aggregator/aggregator_multiton.hpp"
#include "statistics_log/statlog_multiton.hpp"
#include "system_statistics/system_statistics_multiton.hpp"
#include "statistics_gateway/statistics_gateway_multiton.hpp"
#include "statistics_service/statistics_service_multiton.hpp"
#include "statistics_service/statistics_service_compact_multiton.hpp"
#include <wfc/module/component_list.hpp>
#include <wfc/name.hpp>

namespace wfc{ namespace core{

namespace
{
  WFC_NAME2(module_name, "statistics")

  class impl: public ::wfc::component_list<
    module_name
    , statistics_multiton
    , aggregator_multiton
    , system_statistics_multiton
    , statlog_multiton
    , statistics_gateway_multiton
    , statistics_service_multiton
    , statistics_service_compact_multiton
  >
  {
    virtual std::string description() const override
    {
      return "Collect, aggregate and send statistics to the BTP";
    }
  };
}

statistics_module::statistics_module()
  : module( std::make_shared<impl>() )
{
}

}}
