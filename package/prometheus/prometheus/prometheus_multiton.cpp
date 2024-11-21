
#include "prometheus_multiton.hpp"
#include "prometheus_domain.hpp"
#include "prometheus_config_json.hpp"
#include <wfc/module/multiton.hpp>
#include <wfc/module/instance.hpp>
#include <wfc/name.hpp>

namespace wfc{ namespace core{

namespace {

  WFC_NAME2(object_name, "prometheus")

  class impl: public ::wfc::multiton<
    object_name,
      ::wfc::instance<prometheus_domain>,
      prometheus_config_json,
      component_features::DisabledSuspend 
    >
  {
  public:
    virtual std::string interface_name() const override
    {
      return std::string("");
    }
  
    virtual std::string description() const override
    {
      return "prometheus-like /metrics endpoint";
    }
  };
}

prometheus_multiton::prometheus_multiton()
  : component( std::make_shared<impl>() )
{}

}}
