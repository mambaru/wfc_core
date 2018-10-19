//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2013-2018
//
// Copyright: See COPYING file that comes with this distribution
//

#pragma once

#include <wfc/json.hpp>
#include "add_deprecated.hpp"

namespace wfc { namespace btp {

namespace request {
  
  struct add_deprecated_json
  {
    JSON_NAME(avg)
    JSON_NAME(count)
    JSON_NAME(perc50)
    JSON_NAME(perc80)
    JSON_NAME(perc95)
    JSON_NAME(perc99)
    JSON_NAME(perc100)

    typedef add_deprecated::aggregated aggregated;
    typedef add_deprecated::counter_t counter_t;
    typedef add_deprecated::mictotime_t mictotime_t;
    typedef add_deprecated::counter_list counter_list;
    
    typedef json::object<
      aggregated,
      fas::type_list_n<
         json::member<n_avg, aggregated, counter_t,  &aggregated::avg   >
        ,json::member<n_count,    aggregated, counter_t,  &aggregated::count >
        ,json::member<n_perc50,   aggregated, counter_t,  &aggregated::perc50>
        ,json::member<n_perc80,   aggregated, counter_t,  &aggregated::perc80>
        ,json::member<n_perc95,   aggregated, counter_t,  &aggregated::perc95>
        ,json::member<n_perc99,   aggregated, counter_t,  &aggregated::perc99>
        ,json::member<n_perc100,   aggregated, counter_t,  &aggregated::perc100>
      >::type
    > aggregated_json;

    typedef json::array< std::vector< json::value<counter_t> > > counter_list_json;

    JSON_NAME(ts)
    JSON_NAME(name)
    JSON_NAME(ag)
    JSON_NAME(cl)
 
    typedef json::object<
      add_deprecated,
      json::member_list<
        json::member<n_ts,   add_deprecated, mictotime_t, &add_deprecated::ts>,
        json::member<n_name, add_deprecated, std::string, &add_deprecated::name>,
        json::member<n_ag,   add_deprecated, aggregated, &add_deprecated::ag, aggregated_json>,
        json::member<n_cl,   add_deprecated, counter_list, &add_deprecated::cl, counter_list_json >
      >
    > type;
    
    typedef type::target target;
    typedef type::serializer serializer;
  };
}

namespace response
{
  struct add_deprecated_json
  {
    FAS_NAME(result)
    
    typedef wfc::json::object<
      add_deprecated,
      fas::type_list_n<
        wfc::json::member<n_result, add_deprecated, bool, &add_deprecated::result>
      >::type
    > type;

    typedef type::target target;
    typedef type::serializer serializer;
  };
}

}}
