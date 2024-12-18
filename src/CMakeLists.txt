find_package(Clingo REQUIRED)

add_library(domainddm SHARED ddm.c clingo_solver.c)
target_link_libraries(domainddm PRIVATE clingo resources)

add_subdirectory(lp)
