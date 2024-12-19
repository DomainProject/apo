# - Try to find Clingo
# Once done, this will define
#
#  Clingo_FOUND - system has Clingo
#  Clingo_INCLUDE_DIRS - the Clingo include directories
#  Clingo_LIBRARIES - link these to use Clingo

include(LibFindMacros)

# Use pkg-config to get hints about paths
libfind_pkg_check_modules(Clingo_PKGCONF clingo)

# Include dir
find_path(Clingo_INCLUDE_DIR
        NAMES clingo.h
        PATHS ${Clingo_PKGCONF_INCLUDE_DIRS}
)

# Finally the library itself
find_library(Clingo_LIBRARY
        NAMES clingo
        PATHS ${Clingo_PKGCONF_LIBRARY_DIRS}
)

# Set the include dir variables and the libraries and let libfind_process do the rest.
# NOTE: Singular variables for this library, plural for libraries this this lib depends on.
set(Clingo_PROCESS_INCLUDES Clingo_INCLUDE_DIR)
set(Clingo_PROCESS_LIBS Clingo_LIBRARY)
libfind_process(Clingo)
