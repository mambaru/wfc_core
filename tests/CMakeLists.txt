enable_testing()

<<<<<<< HEAD
<<<<<<< HEAD

include_directories(../core)

add_executable(core_test core_test.cpp ../core/core.cpp ../core/detail/po.cpp)
add_executable(wfc_core_test wfc_core_test.cpp )

#link_directories(../../comet/build/comet)
#link_directories(/home/migashko/prj/gitlab.lan/comet/build/comet/)
#link_directories(/home/migashko/prj/gitlab.lan/comet/build/)
#find_library(COMET NAMES comet PATHS /home/migashko/prj/gitlab.lan/comet/build/comet/ NO_DEFAULT_PATH)
#find_library(COMET NAMES comet PATHS ../../comet/build/comet NO_DEFAULT_PATH)

#message(STATUS ${COMET} )

target_link_libraries(core_test wfc )
target_link_libraries(wfc_core_test wfc )
target_link_libraries(wfc_core_test wfc_core )

add_test(core_test core_test)
add_test(wfc_core_test wfc_core_test)

include_directories(../startup)

add_executable(startup_test startup_test.cpp ../startup/startup.cpp ../startup/detail/po.cpp)

target_link_libraries(startup_test wfc )

add_test(startup_test startup_test)

=======
include_directories(../config)

add_executable(config_test config_test.cpp ../config/config.cpp )
target_link_libraries(config_test wfc)

#link_directories(../../wfc/build/wfc)
#link_directories(/home/migashko/prj/gitlab.lan/wfc/build/wfc/)
#link_directories(/home/migashko/prj/gitlab.lan/wfc/build/)
#find_library(COMET NAMES wfc PATHS /home/migashko/prj/gitlab.lan/wfc/build/wfc/ NO_DEFAULT_PATH)
#find_library(COMET NAMES wfc PATHS ../../wfc/build/wfc NO_DEFAULT_PATH)

message(STATUS ${COMET} )


add_test(config_test config_test)
>>>>>>> fc238025937863890c08c73c4b0923421fd81f0a
=======
include_directories(../logger)

find_package( Threads )

add_executable(logger_module_test logger_module_test.cpp ../logger/logger.cpp )
target_link_libraries(logger_module_test wfc ${CMAKE_THREAD_LIBS_INIT} )

add_test(logger_module_test logger_module_test)
>>>>>>> 1c06975c5bf67c94145d2db59b4cddc80ed1843c
