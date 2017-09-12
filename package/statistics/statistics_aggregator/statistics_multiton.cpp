
#include "statistics_multiton.hpp"
#include "statistics_domain.hpp"
#include "statistics_config_json.hpp"
#include <wfc/module/multiton.hpp>
#include <wfc/module/instance.hpp>
#include <wfc/statistics/stat_options_json.hpp>
#include <wfc/name.hpp>

namespace wfc{ namespace core{

namespace {

  WFC_NAME2(object_name, "statistics-aggregator")

  class impl: public ::wfc::multiton<
    object_name,
    ::wfc::instance<statistics_domain>,
    statistics_config_json,
    ::wfc::component_features::Defaults,
    stat_options_json
  >
  {
  public:
    virtual std::string interface_name() override
    {
      return std::string("wfc::iinterafce");
    }
  };
}

statistics_multiton::statistics_multiton()
  : component( std::make_shared<impl>() )
{}

}}
