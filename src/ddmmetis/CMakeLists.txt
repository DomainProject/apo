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
target_link_libraries(metis_utils PRIVATE ${Metis_LIBRARY})



add_custom_command(TARGET metis_utils PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
        ${CMAKE_CURRENT_SOURCE_DIR} $<TARGET_FILE_DIR:metis_utils>)

add_custom_target(copy_assets_new
        COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}
)

add_custom_command(
        OUTPUT metis_utils.o
        COMMAND ld -r -b binary -o metis_utils.o ${metis_utils}
        DEPENDS copy_assets_new)
