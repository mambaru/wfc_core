enable_testing()


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

