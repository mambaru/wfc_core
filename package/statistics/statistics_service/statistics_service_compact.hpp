//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2013-2018
//
// Copyright: See COPYING file that comes with this distribution
//

#pragma once

#include <wrtstat/api/push_json_compact.hpp>
#include <wrtstat/api/multi_push_json_compact.hpp>
#include <wrtstat/api/del_json_compact.hpp>
#include <wfc/statistics/istatistics.hpp>
#include <wfc/jsonrpc.hpp>

namespace wfc{ namespace core{ namespace statistics{

namespace compact{
  
using namespace ::wfc::statistics;
using namespace ::wrtstat;

JSONRPC_TAG(push)
JSONRPC_TAG(multi_push)
JSONRPC_TAG(del)

struct service_method_list: jsonrpc::method_list
<
  jsonrpc::target<istatistics>,
  jsonrpc::invoke_method< _push_, request::push_json_compact, response::push_json_compact, istatistics, &istatistics::push>,
  jsonrpc::invoke_method< _multi_push_, request::multi_push_json_compact, response::multi_push_json_compact, istatistics, &istatistics::multi_push>,
  jsonrpc::invoke_method< _del_, request::del_json_compact, response::del_json_compact, istatistics, &istatistics::del>
>
{};

}}}}
