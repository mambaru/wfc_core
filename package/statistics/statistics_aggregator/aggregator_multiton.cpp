
#include "aggregator_multiton.hpp"
#include "aggregator_domain.hpp"
#include "aggregator_config_json.hpp"
#include <wfc/module/multiton.hpp>
#include <wfc/module/instance.hpp>
#include <wrtstat/wrtstat_options_json.hpp>
#include <wfc/name.hpp>

namespace wfc{ namespace core{

namespace {

  WFC_NAME2(object_name, "statistics-aggregator")

  class impl: public multiton<
    object_name,
    instance<aggregator_domain>,
    aggregator_config_json,
    component_features::Defaults,
    aggregator_statistics_config_json
  >
  {
  public:
    virtual std::string interface_name() const override
    {
      return std::string("istatistics");
    }

    virtual std::string description() const override
    {
      return "Real-time statistics aggregator";
    }
  };
}

aggregator_multiton::aggregator_multiton()
  : component( std::make_shared<impl>() )
{}

}}
