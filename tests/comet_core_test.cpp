#include "core_module.hpp"
#include <comet/comet.hpp>

using namespace ::mamba::comet;

int main(int argc, char* argv[])
{
  comet({
    {"core", std::make_shared<core_module>()}
  }).run(argc, argv);

}