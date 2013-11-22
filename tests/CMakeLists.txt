enable_testing()

include_directories(../core)

add_executable(core_test core_test.cpp ../core/core.cpp ../core/detail/po.cpp)
add_executable(comet_core_test comet_core_test.cpp )

#link_directories(../../comet/build/comet)
#link_directories(/home/migashko/prj/gitlab.lan/comet/build/comet/)
#link_directories(/home/migashko/prj/gitlab.lan/comet/build/)
#find_library(COMET NAMES comet PATHS /home/migashko/prj/gitlab.lan/comet/build/comet/ NO_DEFAULT_PATH)
#find_library(COMET NAMES comet PATHS ../../comet/build/comet NO_DEFAULT_PATH)

#message(STATUS ${COMET} )

target_link_libraries(comet_core_test comet )
target_link_libraries(comet_core_test comet_core )

add_test(core_test core_test)
add_test(comet_core_test comet_core_test)