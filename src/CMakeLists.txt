find_package(Clingo REQUIRED)

add_library(clingo_solver SHARED clingo_solver.c)
target_link_libraries(clingo_solver PRIVATE clingo)

add_executable(ddm_test ddm_driver.c ddm.c)
target_link_libraries(ddm_test PRIVATE clingo_solver clingo resources)

add_subdirectory(lp)
