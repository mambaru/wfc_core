enable_testing()

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
