
# If using non-standard paths for clingo, set these variables to the appropriate paths
#set(Clingo_INCLUDE_DIR "/custom/path")
#set(Clingo_LIBRARY "/custom/path")

find_package(Clingo REQUIRED)
message(STATUS "Clingo include dirs: ${Clingo_INCLUDE_DIRS}")
message(STATUS "Clingo libraries: ${Clingo_LIBRARIES}")

# Find the Metis library
set(USER_HOME_DIRECTORY $ENV{HOME})
message(STATUS "User Home Directory: ${USER_HOME_DIRECTORY}")
set(Metis_INCLUDE_DIR $ENV{HOME}/local/include)
set(Metis_LIBRARY $ENV{HOME}/local/lib/libmetis.so)
set(GKlib_LIBRARY $ENV{HOME}/local/lib/libGKlib.a)

#find_package(Metis REQUIRED)
message(STATUS "Metis include dirs: ${Metis_INCLUDE_DIR}")
message(STATUS "Metis libraries: ${Metis_LIBRARY}")
message(STATUS "GKLib libraries: ${GKlib_LIBRARY}")


add_library(domainddm SHARED ddm.c dynstr.c clingo_solver.c)
target_include_directories(domainddm PRIVATE ${Clingo_INCLUDE_DIRS} ${Metis_INCLUDE_DIR})
target_link_libraries(domainddm PRIVATE clingo resources ddmmetis)


add_library(ddmmetis SHARED ./ddmmetis/ddmmetis.c ./ddmmetis/utils.c)
target_include_directories(ddmmetis PRIVATE ${Metis_INCLUDE_DIR})
target_link_libraries(ddmmetis PRIVATE ${Metis_LIBRARY} ${GKlib_LIBRARY} resources)

add_subdirectory(lp)
#add_subdirectory(ddmmetis)
