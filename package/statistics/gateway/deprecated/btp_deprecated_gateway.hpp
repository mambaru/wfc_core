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
  
  virtual void add(request::add::ptr req, response::add::handler cb ) override
  {
    if ( req==nullptr )
    {
      if ( cb!=nullptr )
        cb(nullptr);
      return;
    }
    
    auto depr_add = std::make_unique<request::add_deprecated>();
    depr_add->ts = req->ts;
    depr_add->name = std::move( req->name);
    depr_add->cl = std::move( req->data );
    depr_add->ag.avg     = req->avg;
    depr_add->ag.count   = req->count;
    depr_add->ag.perc50  = req->perc50;
    depr_add->ag.perc80  = req->perc80;
    depr_add->ag.perc95  = req->perc95;
    depr_add->ag.perc99  = req->perc99;
    depr_add->ag.perc100 = req->perc100;

    response::add_deprecated::handler depr_cb = nullptr;
    if ( cb!=nullptr )
    {
      depr_cb = [cb](response::add_deprecated::ptr res)
      {
        auto orig_res = std::make_unique<response::add>();
        if (res!=nullptr)
          orig_res->result = res->result;
        if (cb!=nullptr)
          cb( std::move(orig_res) );
      };
    }
      
    this->template call< _add_ >( 
      std::move(depr_add), 
      std::move(depr_cb),
      nullptr
    );
  }
  
};

}}}
