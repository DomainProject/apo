# Find Metis package
find_package(Metis REQUIRED)

# Define the source files for your library
set(metis_sources
    ddmmetis.c utils.c
)

# Add a library or executable for your Metis-related files
add_library(metis_utils STATIC ${metis_sources})
set_target_properties(metis_utils PROPERTIES LINKER_LANGUAGE C)

# Include Metis headers
target_include_directories(metis_utils PRIVATE ${Metis_INCLUDE_DIR})


# Link the Metis library to this target
target_link_libraries(metis_utils PRIVATE ${Metis_LIBRARY} m)
