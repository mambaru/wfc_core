enable_testing()

include_directories(../startup)

add_executable(startup_test startup_test.cpp ../startup/startup.cpp ../startup/detail/po.cpp)

target_link_libraries(startup_test wfc )

add_test(startup_test startup_test)
