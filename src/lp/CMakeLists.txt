# SPDX-FileCopyrightText: 2023-2025 Alessandro Pellegrini <a.pellegrini@ing.uniroma2.it>
# SPDX-License-Identifier: GPL-3.0-only

set(resources
        ddm.asp
)

add_library(resources STATIC resources.c)

add_executable(embedfiles embedfiles.c)

add_custom_target(copy_assets
        COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}
)

add_custom_command(
        OUTPUT resources.c
        COMMAND embedfiles ${resources}
        DEPENDS copy_assets)



#set_target_properties(resources PROPERTIES LINKER_LANGUAGE C)
#target_include_directories(resources PRIVATE .)
#
#add_custom_command(TARGET resources PRE_BUILD
#        COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
#        ${CMAKE_CURRENT_SOURCE_DIR} $<TARGET_FILE_DIR:resources>)
#
#
#
#add_custom_command(
#        OUTPUT resources.o
#        COMMAND ld -r -b binary -o resources.o ${resources}
#        DEPENDS copy_assets)
