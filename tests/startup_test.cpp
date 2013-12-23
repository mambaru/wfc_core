#include "startup.hpp"

using namespace ::wfc;

int main()
{
  std::shared_ptr< wfc::global > g;
  startup_impl c( g );

}