enable_testing()

include_directories(../config)

add_executable(config_test config_test.cpp ../config/config.cpp )
target_link_libraries(config_test comet)

#link_directories(../../comet/build/comet)
#link_directories(/home/migashko/prj/gitlab.lan/comet/build/comet/)
#link_directories(/home/migashko/prj/gitlab.lan/comet/build/)
#find_library(COMET NAMES comet PATHS /home/migashko/prj/gitlab.lan/comet/build/comet/ NO_DEFAULT_PATH)
#find_library(COMET NAMES comet PATHS ../../comet/build/comet NO_DEFAULT_PATH)

message(STATUS ${COMET} )


add_test(config_test config_test)
