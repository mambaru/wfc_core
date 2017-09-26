#pragma once

#include <wfc/statistics/ibtp.hpp>
#include <wfc/jsonrpc.hpp>
#include "add_deprecated_json.hpp"


namespace wfc{ namespace core{ namespace gateway{ 

using namespace ::wfc::btp;

JSONRPC_TAG(add)

struct btp_deprecated_method_list: public ::wfc::jsonrpc::method_list
<
  ::wfc::jsonrpc::interface_<ibtp>,
  ::wfc::jsonrpc::call_method< _add_, request::add_deprecated_json, response::add_deprecated_json>
>
{};

template<typename Base>
class btp_deprecated_interface
  : public Base
{
public:
  
  virtual void add(request::add::ptr /*req*/, response::add::handler cb ) override
  {
    auto depr_add = std::make_unique<request::add_deprecated>();
#error need init depr_add from req
    response::add_deprecated::handler depr_cb 
      = [cb](response::add_deprecated::ptr res)
      {
        auto orig_res = std::make_unique<response::add>();
        if (res!=nullptr)
          orig_res->result = res->result;
        if (cb!=nullptr)
          cb( std::move(orig_res) );
      };
      
    this->template call< _add_ >( 
      std::move(depr_add), 
      std::move(depr_cb),
      nullptr
    );
  }
  
};

}}}
