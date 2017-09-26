#pragma once
#include <wrtstat/aggregated_data.hpp>
#include <wrtstat/types.hpp>

namespace wfc{ namespace btp{

namespace request
{
  struct add_deprecated
  {
    typedef long long int counter_t;
    typedef std::vector<counter_t>  counter_list;
    typedef std::time_t mictotime_t;
    struct aggregated
    {
      counter_t avg = 0;
      counter_t count = 0;
      counter_t perc50 = 0;
      counter_t perc80 = 0;
      counter_t perc95 = 0;
      counter_t perc99 = 0;
      counter_t perc100 = 0;
    };
    
    mictotime_t ts = 0;
    std::string name;
    aggregated ag;
    counter_list cl;

    typedef std::unique_ptr<add_deprecated> ptr;
  };
}

namespace response
{
  struct add_deprecated
  {
    bool result = true;
    typedef std::unique_ptr<add_deprecated> ptr;
    typedef std::function< void(ptr)> handler;
  };
}

}}
