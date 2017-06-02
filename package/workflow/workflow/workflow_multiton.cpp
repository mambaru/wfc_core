
#include "workflow_multiton.hpp"
#include "workflow_domain.hpp"
#include "workflow_config_json.hpp"
#include <wfc/module/multiton.hpp>
#include <wfc/module/instance.hpp>
#include <wfc/name.hpp>

namespace wfc{ namespace core{

namespace {

  WFC_NAME2(object_name, "workflow")

  class impl: public ::wfc::multiton<
    object_name,
    ::wfc::instance<workflow_domain>,
    workflow_config_json,
      component_features::DisabledSuspend 
    | component_features::DisabledWorkflow
    | component_features::EnableCPU, 
    workflow_statistics_json
  >
  {};
}

workflow_multiton::workflow_multiton()
  : component( std::make_shared<impl>() )
{}

}}
