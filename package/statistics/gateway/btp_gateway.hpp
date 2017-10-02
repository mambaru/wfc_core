#pragma once

#include <wfc/statistics/ibtp.hpp>
#include <wfc/statistics/api/push_json.hpp>
#include <wfc/jsonrpc.hpp>


namespace wfc{ namespace core{ namespace gateway{ 

using namespace ::wfc::btp;

JSONRPC_TAG(push)

struct btp_method_list: public ::wfc::jsonrpc::method_list
<
  ::wfc::jsonrpc::interface_<ibtp>,
  ::wfc::jsonrpc::call_method< _push_, request::push_json, response::push_json>
>
{};

template<typename Base>
class btp_interface
  : public Base
{
public:
  
  virtual void push(request::push::ptr req, response::push::handler cb ) override
  {
    this->template call< _push_ >( std::move(req), cb, nullptr);
  }
  
};

}}}
