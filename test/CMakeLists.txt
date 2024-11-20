add_executable(workload workload.c)
target_link_libraries(workload PRIVATE clingo_solver clingo resources)
target_include_directories(workload PRIVATE ${CMAKE_SOURCE_DIR}/src)
