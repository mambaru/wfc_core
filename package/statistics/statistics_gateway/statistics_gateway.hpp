//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2013-2018
//
// Copyright: See COPYING file that comes with this distribution
//

#pragma once

#include <wfc/statistics/istatistics.hpp>
#include <wfc/statistics/api/push_json.hpp>
#include <wfc/statistics/api/del_json.hpp>
#include <wfc/jsonrpc.hpp>


namespace wfc{ namespace core{ namespace statistics{ 

using namespace ::wfc::statistics;

JSONRPC_TAG(push)
JSONRPC_TAG(del)

struct gateway_method_list: public ::wfc::jsonrpc::method_list
<
  ::wfc::jsonrpc::interface_<istatistics>,
  ::wfc::jsonrpc::call_method< _push_, request::push_json, response::push_json>,
  ::wfc::jsonrpc::call_method< _del_, request::del_json, response::del_json>
>
{};

template<typename Base>
class gateway_interface
  : public Base
{
public:
  
  virtual void push(request::push::ptr req, response::push::handler cb ) override
  {
    this->template call< _push_ >( std::move(req), cb, nullptr);
  }

  virtual void del(request::del::ptr req, response::del::handler cb ) override
  {
    this->template call< _del_ >( std::move(req), cb, nullptr);
  }

};

}}}
