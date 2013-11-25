enable_testing()

include_directories(../logger)

add_executable(logger_module_test logger_module_test.cpp ../logger/logger.cpp )
add_test(logger_module_test logger_module_test)
