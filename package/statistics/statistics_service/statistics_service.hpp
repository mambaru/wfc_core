#pragma once


#include <wfc/statistics/api/push_json.hpp>
#include <wfc/statistics/api/del_json.hpp>
#include <wfc/statistics/istatistics.hpp>
#include <wfc/jsonrpc.hpp>

namespace wfc{ namespace core{ namespace statistics{

JSONRPC_TAG(push)
JSONRPC_TAG(del)

struct service_method_list: wfc::jsonrpc::method_list
<
  wfc::jsonrpc::target< wfc::istatistics>,
  wfc::jsonrpc::invoke_method< _push_, wfc::statistics::request::push_json, wfc::statistics::response::push_json, wfc::istatistics, &wfc::istatistics::push>,
  wfc::jsonrpc::invoke_method< _del_, wfc::statistics::request::del_json, wfc::statistics::response::del_json, wfc::istatistics, &wfc::istatistics::del>
>
{};

}}}
