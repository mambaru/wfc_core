enable_testing()

include_directories(../core)

add_executable(core_test core_test.cpp ../core/core.cpp)
add_test(core_test core_test)