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
set(resources5 ddm_c5.asp)
set(resources6 ddm_c6.asp)
set(resources7 ddm_c7.asp)


add_library(resources STATIC resources.c)
add_library(resources_c1 STATIC resources1.c)
add_library(resources_c2 STATIC resources2.c)
add_library(resources_c3 STATIC resources3.c)
add_library(resources_c4 STATIC resources4.c)
add_library(resources_c5 STATIC resources5.c)
add_library(resources_c6 STATIC resources6.c)
add_library(resources_c7 STATIC resources7.c)



add_custom_command(
        OUTPUT resources.c
        COMMAND embedfiles ${resources}
        COMMAND mv ${resources} resources.c.bck
        DEPENDS copy_assets ${resources} embedfiles)

add_custom_command(
        OUTPUT resources1.c
        COMMAND embedfiles ${resources1}
        COMMAND mv resources.c resources1.c
        COMMAND sed -i 's/ddm_c1/ddm/g' resources1.c
        DEPENDS copy_assets ${resources1} embedfiles)

add_custom_command(
        OUTPUT resources2.c
        COMMAND embedfiles ${resources2}
        COMMAND mv resources.c resources2.c
        COMMAND sed -i 's/ddm_c2/ddm/g' resources2.c
        DEPENDS copy_assets ${resources2} embedfiles)

add_custom_command(
        OUTPUT resources3.c
        COMMAND embedfiles ${resources3}
        COMMAND mv resources.c resources3.c
        COMMAND sed -i 's/ddm_c3/ddm/g' resources3.c
        DEPENDS copy_assets ${resources3} embedfiles)


add_custom_command(
        OUTPUT resources4.c
        COMMAND embedfiles ${resources4}
        COMMAND mv resources.c resources4.c
        COMMAND sed -i 's/ddm_c4/ddm/g' resources4.c
        DEPENDS copy_assets ${resources4} embedfiles)


add_custom_command(
        OUTPUT resources5.c
        COMMAND embedfiles ${resources5}
        COMMAND mv resources.c resources5.c
        COMMAND sed -i 's/ddm_c5/ddm/g' resources5.c
        DEPENDS copy_assets ${resources5} embedfiles)

add_custom_command(
        OUTPUT resources6.c
        COMMAND embedfiles ${resources6}
        COMMAND mv resources.c resources6.c
        COMMAND sed -i 's/ddm_c6/ddm/g' resources6.c
        DEPENDS copy_assets ${resources6} embedfiles)

add_custom_command(
        OUTPUT resources7.c
        COMMAND embedfiles ${resources7}
        COMMAND mv resources.c resources7.c
        COMMAND sed -i 's/ddm_c7/ddm/g' resources7.c
        DEPENDS copy_assets ${resources7} embedfiles)
