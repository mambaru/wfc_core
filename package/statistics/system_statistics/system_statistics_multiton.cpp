
#include "system_statistics_multiton.hpp"
#include "system_statistics_domain.hpp"
#include "system_statistics_config_json.hpp"
#include "system_statistics_options_json.hpp"
#include <wfc/module/multiton.hpp>
#include <wfc/module/instance.hpp>
#include <wfc/name.hpp>

namespace wfc{  namespace core{

namespace {

  WFC_NAME2(object_name, "system-statistics")

  class impl: public ::wfc::multiton<
    object_name,
    ::wfc::instance<system_statistics_domain>,
    system_statistics_config_json,
    ::wfc::component_features::Defaults,
    system_statistics_options_json
  >
  {};
}

system_statistics_multiton::system_statistics_multiton()
  : component( std::make_shared<impl>() )
{}

}}
