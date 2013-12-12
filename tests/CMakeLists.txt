enable_testing()

include_directories(../logger)

add_executable(logger_module_test logger_module_test.cpp ../logger/logger.cpp )
target_link_libraries(logger_module_test wfc)
add_test(logger_module_test logger_module_test)
