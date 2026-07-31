find_package(Clingo REQUIRED)
message(STATUS "Clingo include dirs: ${Clingo_INCLUDE_DIRS}")
message(STATUS "Clingo libraries: ${Clingo_LIBRARIES}")

add_library(aspsolver STATIC asp_solver.c)
target_include_directories(aspsolver PUBLIC ${Clingo_INCLUDE_DIRS})
target_link_libraries(aspsolver PUBLIC ${Clingo_LIBRARIES})
