
# If using non-standard paths for clingo, set these variables to the appropriate paths
#set(Clingo_INCLUDE_DIR "/custom/path")
#set(Clingo_LIBRARY "/custom/path")
find_package(Clingo REQUIRED)
message(STATUS "Clingo include dirs: ${Clingo_INCLUDE_DIRS}")
message(STATUS "Clingo libraries: ${Clingo_LIBRARIES}")

add_library(domainddm SHARED ddm.c dynstr.c clingo_solver.c)
target_link_libraries(domainddm PRIVATE clingo resources)
target_include_directories(domainddm PRIVATE ${Clingo_INCLUDE_DIRS})

add_subdirectory(lp)
