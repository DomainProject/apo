# SPDX-FileCopyrightText: 2023-2025 Alessandro Pellegrini <a.pellegrini@ing.uniroma2.it>
# SPDX-License-Identifier: GPL-3.0-only

set(res_files
        ddm.asp
)

add_library(resources STATIC resources.c)

add_executable(embedfiles embedfiles.c)

add_custom_target(copy_assets
        COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}
)

add_custom_command(
        OUTPUT resources.c
        COMMAND embedfiles ${res_files}
        DEPENDS copy_assets ${res_files} embedfiles)
