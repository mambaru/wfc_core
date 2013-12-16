#include "logger.hpp"

#include <fas/algorithm.hpp>
#include <fas/integral.hpp>
#include <fas/type_list.hpp>
using namespace ::wfc;

typedef fas::generate<
  fas::int_<10>,
  fas::generator<
    fas::rand<int_<111> >,
    fas::make_int< fas::rand<_> >
  >
>::type randlist;

typedef fas::for_<
  fas::pair< fas::int_<0>,  randlist>, 
  is_empty_list< fas::second< fas::_ > >, 
  fas::pair<
    fas::plus< fas::first<fas::_>, fas::second< fas::head<fas::_> > >
  >
  
>::type1 type;

int main()
{
  return 0;
}