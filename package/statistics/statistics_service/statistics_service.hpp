//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2013-2018
//
// Copyright: See COPYING file that comes with this distribution
//

#pragma once

#include <wrtstat/api/push_json.hpp>
#include <wrtstat/api/multi_push_json.hpp>
#include <wrtstat/api/del_json.hpp>
#include <wfc/statistics/istatistics.hpp>
#include <wfc/jsonrpc.hpp>

namespace wfc{ namespace core{ namespace statistics{

using namespace ::wfc::statistics;
using namespace ::wrtstat;

JSONRPC_TAG(push)
JSONRPC_TAG(multi_push)
JSONRPC_TAG(del)

struct service_method_list: jsonrpc::method_list
<
  jsonrpc::target<istatistics>,
  jsonrpc::invoke_method< _push_, request::push_json, response::push_json, istatistics, &istatistics::push>,
  jsonrpc::invoke_method< _multi_push_, request::multi_push_json, response::multi_push_json, istatistics, &istatistics::multi_push>,
  jsonrpc::invoke_method< _del_, request::del_json, response::del_json, istatistics, &istatistics::del>
>
{};

}}}
