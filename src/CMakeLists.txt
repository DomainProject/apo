
# If using non-standard paths for clingo, set these variables to the appropriate paths
#set(Clingo_INCLUDE_DIR "/custom/path")
#set(Clingo_LIBRARY "/custom/path")
find_package(Clingo REQUIRED)

add_library(clingo_solver SHARED clingo_solver.c)
target_link_libraries(clingo_solver PRIVATE clingo)
target_include_directories(clingo_solver PRIVATE ${Clingo_INCLUDE_DIRS})

add_executable(ddm_test ddm_driver.c ddm.c)
target_link_libraries(ddm_test PRIVATE clingo_solver clingo resources)
target_include_directories(ddm_test PRIVATE ${Clingo_INCLUDE_DIRS})
message(STATUS "Clingo include dirs: ${Clingo_INCLUDE_DIRS}")
message(STATUS "Clingo libraries: ${Clingo_LIBRARIES}")

add_subdirectory(lp)
