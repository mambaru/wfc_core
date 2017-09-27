
#include "statlog_multiton.hpp"
#include "statlog_domain.hpp"
#include "statlog_config_json.hpp"
#include <wfc/module/multiton.hpp>
#include <wfc/module/instance.hpp>
#include <wfc/name.hpp>

namespace wfc{ namespace core{

namespace {

  WFC_NAME2(object_name, "statistic-log")

  class impl: public ::wfc::multiton<
    object_name,
    ::wfc::instance<statlog_domain>,
    statlog_config_json,
    ::wfc::component_features::Defaults
  >
  {
  public:
    virtual std::string interface_name() const override
    {
      return std::string("ibtp");
    }

    virtual std::string description() const override
    {
      return "Write aggregated data into user log";
    }
  };
}

statlog_multiton::statlog_multiton()
  : component( std::make_shared<impl>() )
{}

}}
