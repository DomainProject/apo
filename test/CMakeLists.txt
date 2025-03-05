add_executable(workload workload.c)
target_link_libraries(workload PRIVATE domainddm)
target_include_directories(workload PRIVATE ${CMAKE_SOURCE_DIR}/src)


add_executable(ddm_stress_test ddm_stress_test.c)
target_link_libraries(ddm_stress_test PRIVATE domainddm)

add_executable(ddm_test ddm_test.c)
target_link_libraries(ddm_test PRIVATE domainddm)

add_executable(metis_test metis_test.c)
target_link_libraries(metis_test PRIVATE ddmmetis)
