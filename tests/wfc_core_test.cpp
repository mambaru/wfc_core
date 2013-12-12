#include "core_module.hpp"
#include <wfc/wfc.hpp>
#include <memory>
//using namespace ::wfc;

int main(int argc, char* argv[])
{
  wfc::wfc("",  {
    {"core", std::make_shared< wfc::core_module >()}
  }).run(argc, argv);

}