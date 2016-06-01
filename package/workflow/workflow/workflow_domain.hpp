#pragma once

#include <wfc/domain_object.hpp>
#include <wfc/core/workflow.hpp>
#include <wfc/core/workflow_options.hpp>
#include <string>
#include <memory>

namespace wfc{

class workflow_domain
  : public domain_object<iinterface, workflow_options>
{
  class impl;
public:
  typedef ::wfc::workflow domain_interface;
  typedef ::wfc::workflow_options options_type;
  virtual ~workflow_domain();
  using domain_object<iinterface, workflow_options>::create;
  virtual void create() override;
  virtual void reconfigure() override;
  virtual void start(const std::string& ) override;
  virtual void stop(const std::string& ) override;
private:
  std::shared_ptr<impl> _workflow;
};

}
