add_executable(workload workload.c)
target_link_libraries(workload PRIVATE domainddm clingo)
target_include_directories(workload PRIVATE ${CMAKE_SOURCE_DIR}/src)

add_executable(ddm_test ddm_test.c)
add_executable(ddm_test2 ddm_test2.c)
target_link_libraries(ddm_test PRIVATE domainddm clingo)
target_link_libraries(ddm_test2 PRIVATE domainddm clingo)
