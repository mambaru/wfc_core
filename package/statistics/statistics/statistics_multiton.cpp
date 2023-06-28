
#include "statistics_multiton.hpp"
#include "statistics_domain.hpp"
#include "statistics_config_json.hpp"
#include <wfc/module/multiton.hpp>
#include <wfc/module/instance.hpp>
#include <wrtstat/wrtstat_options_json.hpp>
#include <wfc/name.hpp>

namespace wfc{ namespace core{

namespace {

  WFC_NAME2(object_name, "statistics")

  class impl: public multiton<
    object_name,
    instance<statistics_domain>,
    statistics_config_json,
    component_features::Defaults,
    statistics_statistics_config_json
  >
  {
  public:
    virtual std::string interface_name() const override
    {
      return std::string("iinterface");
    }

    virtual std::string description() const override
    {
      return "Collects local statistics";
    }
  };
}

statistics_multiton::statistics_multiton()
  : component( std::make_shared<impl>() )
{}

}}
