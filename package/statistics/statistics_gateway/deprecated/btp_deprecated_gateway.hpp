//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2013-2018
//
// Copyright: See COPYING file that comes with this distribution
//

#pragma once

#include <wfc/statistics/istatistics.hpp>
#include <wfc/jsonrpc.hpp>
#include "add_deprecated_json.hpp"


namespace wfc{ namespace core{ namespace gateway{ 

using namespace ::wfc::statistics;

JSONRPC_TAG(add)

struct btp_deprecated_method_list: public ::wfc::jsonrpc::method_list
<
  /*::wfc::jsonrpc::interface_<istatistics>,
  ::wfc::jsonrpc::call_method< _push_, request::add_deprecated_json, response::add_deprecated_json>*/
>
{};

template<typename Base>
class btp_deprecated_interface
  : public Base
{
public:
  /*
  virtual void add(request::add::ptr req, response::add::handler cb ) override
  {
    
    if ( req==nullptr )
    {
      if ( cb!=nullptr )
        cb(nullptr);
      return;
    }
    
    auto depr_push = std::make_unique<request::add_deprecated>();
    depr_push->ts = req->ts;
    depr_push->name = std::move( req->name);
    depr_push->cl = std::move( req->data );
    depr_push->ag.avg     = req->avg;
    depr_push->ag.count   = req->count;
    depr_push->ag.perc50  = req->perc50;
    depr_push->ag.perc80  = req->perc80;
    depr_push->ag.perc95  = req->perc95;
    depr_push->ag.perc99  = req->perc99;
    depr_push->ag.perc100 = req->perc100;

    response::add_deprecated::handler depr_cb = nullptr;
    if ( cb!=nullptr )
    {
      depr_cb = [cb](response::add_deprecated::ptr res)
      {
        auto orig_res = std::make_unique<response::push>();
        if (res!=nullptr)
          orig_res->result = res->result;
        if (cb!=nullptr)
          cb( std::move(orig_res) );
      };
    }
      
    this->template call< _push_ >( 
      std::move(depr_push), 
      std::move(depr_cb),
      nullptr
    );
  }
  */
};

}}}
