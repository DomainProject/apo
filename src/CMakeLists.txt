
set(CMAKE_VERBOSE_MAKEFILE ON)

# If using non-standard paths for clingo, set these variables to the appropriate paths
#set(Clingo_INCLUDE_DIR "/custom/path")
#set(Clingo_LIBRARY "/custom/path")

find_package(Clingo REQUIRED)
message(STATUS "Clingo include dirs: ${Clingo_INCLUDE_DIRS}")
message(STATUS "Clingo libraries: ${Clingo_LIBRARIES}")

# Find the Metis library

#set(USER_HOME_DIRECTORY $ENV{HOME})
#message(STATUS "User Home Directory: ${USER_HOME_DIRECTORY}")
#set(Metis_INCLUDE_DIR $ENV{HOME}/local/include)
#set(Metis_LIBRARY $ENV{HOME}/local/lib/libmetis.so)
#set(GKlib_LIBRARY $ENV{HOME}/local/lib/libGKlib.a)

#set(Metis_INCLUDE_DIR "/usr/local/include")
#set(Metis_LIBRARY "/usr/local/lib/libmetis.so")
find_package(Metis REQUIRED)

message(STATUS "Metis include dirs: ${Metis_INCLUDE_DIR}")
message(STATUS "Metis libraries: ${Metis_LIBRARY}")
#message(STATUS "GKLib libraries: ${GKlib_LIBRARY}")


add_library(domainddm    SHARED ddm.c dynstr.c clingo_solver.c)
#add_library(domainddm_c1 SHARED ddm.c dynstr.c clingo_solver.c)
#add_library(domainddm_c2 SHARED ddm.c dynstr.c clingo_solver.c)
#add_library(domainddm_c3 SHARED ddm.c dynstr.c clingo_solver.c)
#add_library(domainddm_c4 SHARED ddm.c dynstr.c clingo_solver.c)
#add_library(domainddm_c5 SHARED ddm.c dynstr.c clingo_solver.c)
#add_library(domainddm_c6 SHARED ddm.c dynstr.c clingo_solver.c)
#add_library(domainddm_c7 SHARED ddm.c dynstr.c clingo_solver.c)

target_include_directories(domainddm    PUBLIC ${Clingo_INCLUDE_DIRS})
#target_include_directories(domainddm_c1 PUBLIC ${Clingo_INCLUDE_DIRS})
#target_include_directories(domainddm_c2 PUBLIC ${Clingo_INCLUDE_DIRS})
#target_include_directories(domainddm_c3 PUBLIC ${Clingo_INCLUDE_DIRS})
#target_include_directories(domainddm_c4 PUBLIC ${Clingo_INCLUDE_DIRS})
#target_include_directories(domainddm_c5 PUBLIC ${Clingo_INCLUDE_DIRS})
#target_include_directories(domainddm_c6 PUBLIC ${Clingo_INCLUDE_DIRS})
#target_include_directories(domainddm_c7 PUBLIC ${Clingo_INCLUDE_DIRS})

target_link_libraries(domainddm    PRIVATE ${Clingo_LIBRARIES} resources)
#target_link_libraries(domainddm_c1 PRIVATE ${Clingo_LIBRARIES} resources_c1)
#target_link_libraries(domainddm_c2 PRIVATE ${Clingo_LIBRARIES} resources_c2)
#target_link_libraries(domainddm_c3 PRIVATE ${Clingo_LIBRARIES} resources_c3)
#target_link_libraries(domainddm_c4 PRIVATE ${Clingo_LIBRARIES} resources_c4)
#target_link_libraries(domainddm_c5 PRIVATE ${Clingo_LIBRARIES} resources_c5)
#target_link_libraries(domainddm_c6 PRIVATE ${Clingo_LIBRARIES} resources_c6)
#target_link_libraries(domainddm_c7 PRIVATE ${Clingo_LIBRARIES} resources_c7)


add_library(ddmmetis SHARED ./ddmmetis/ddmmetis.c ./ddmmetis/utils.c)
target_include_directories(ddmmetis PUBLIC ${Metis_INCLUDE_DIR})
target_link_libraries(ddmmetis PRIVATE ${Metis_LIBRARY} ${GKlib_LIBRARY})

add_subdirectory(lp)
