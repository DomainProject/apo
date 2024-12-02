cmake_minimum_required(VERSION 3.12)
project("ddm" LANGUAGES C DESCRIPTION "DDM")

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_compile_options(-Wall -Wextra -pedantic -fmacro-prefix-map=${CMAKE_SOURCE_DIR}=)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})
add_subdirectory(src)

# Not a proper testing setup
add_subdirectory(test)
