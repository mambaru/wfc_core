#pragma once

#include <wfc/statistics/ibtp.hpp>
#include <wfc/statistics/api/add_json.hpp>
#include <wfc/jsonrpc.hpp>


namespace wfc{ namespace core{ namespace gateway{ 

using namespace ::wfc::btp;

JSONRPC_TAG(add)

struct btp_method_list: public ::wfc::jsonrpc::method_list
<
  ::wfc::jsonrpc::interface_<ibtp>,
  ::wfc::jsonrpc::call_method< _add_, request::add_json, response::add_json>
>
{};

template<typename Base>
class btp_interface
  : public Base
{
public:
  
  virtual void add(request::add::ptr req, response::add::handler cb ) override
  {
    this->template call< _add_ >( std::move(req), cb, nullptr);
  }
  
};

}}}
