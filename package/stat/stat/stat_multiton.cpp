
#include "stat_multiton.hpp"
#include "stat_domain.hpp"
#include "stat_config_json.hpp"
#include <wfc/module/multiton.hpp>
#include <wfc/module/instance.hpp>
#include <wfc/name.hpp>

namespace wfc{

namespace {

  WFC_NAME2(object_name, "stat")

  class impl: public ::wfc::multiton<
    object_name,
    ::wfc::instance<stat_domain>,
    stat_config_json,
    component_features::DisabledWorkflow 
  >
  {};
}

stat_multiton::stat_multiton()
  : component( std::make_shared<impl>() )
{}

}
