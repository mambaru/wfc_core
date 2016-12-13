
#include "statistics_multiton.hpp"
#include "statistics_domain.hpp"
#include "statistics_config_json.hpp"
#include <wfc/module/multiton.hpp>
#include <wfc/module/instance.hpp>
#include <wfc/name.hpp>

namespace wfc{ namespace core{

namespace {

  WFC_NAME2(object_name, "statistics")

  class impl: public ::wfc::multiton<
    object_name,
    ::wfc::instance<stat_domain>,
    stat_config_json
  >
  {};
}

statistics_multiton::statistics_multiton()
  : component( std::make_shared<impl>() )
{}

}}
