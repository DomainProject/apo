# SPDX-FileCopyrightText: 2023-2025 Alessandro Pellegrini <a.pellegrini@ing.uniroma2.it>
# SPDX-License-Identifier: GPL-3.0-only

add_custom_target(copy_assets
        COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}
)

add_executable(embedfiles embedfiles.c)


set(resources  ddm.asp)
set(resources1 ddm_c1.asp)
set(resources2 ddm_c2.asp)
set(resources3 ddm_c3.asp)
set(resources4 ddm_c4.asp)


add_library(resources STATIC resources.c)
add_library(resources_c1 STATIC resources1.c)
add_library(resources_c2 STATIC resources2.c)
add_library(resources_c3 STATIC resources3.c)
add_library(resources_c4 STATIC resources4.c)


add_custom_command(
        OUTPUT resources.c
        COMMAND embedfiles ${resources}
        COMMAND mv ${resources} resources.c.bck
        DEPENDS copy_assets)

add_custom_command(
        OUTPUT resources1.c
        COMMAND embedfiles ${resources1}
        COMMAND mv resources.c resources1.c
        COMMAND sed -i 's/ddm_c1/ddm/g' resources1.c
        DEPENDS copy_assets)

add_custom_command(
        OUTPUT resources2.c
        COMMAND embedfiles ${resources2}
        COMMAND mv resources.c resources2.c
        COMMAND sed -i 's/ddm_c2/ddm/g' resources2.c
        DEPENDS copy_assets)

add_custom_command(
        OUTPUT resources3.c
        COMMAND embedfiles ${resources3}
        COMMAND mv resources.c resources3.c
        COMMAND sed -i 's/ddm_c3/ddm/g' resources3.c
        DEPENDS copy_assets)


add_custom_command(
        OUTPUT resources4.c
        COMMAND embedfiles ${resources4}
        COMMAND mv resources.c resources4.c
        COMMAND sed -i 's/ddm_c4/ddm/g' resources4.c
        DEPENDS copy_assets ddm_c4.asp)


