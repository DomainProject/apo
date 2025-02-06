# Custom FindMetis.cmake

find_path(Metis_INCLUDE_DIR
  NAMES metis.h
  PATHS /usr/include /usr/local/include
)

find_library(Metis_LIBRARY
        NAMES metis libmetis
        PATHS /usr/lib /usr/local/lib
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Metis
  REQUIRED_VARS Metis_INCLUDE_DIR Metis_LIBRARY
  VERSION_VAR Metis_VERSION
)

mark_as_advanced(Metis_INCLUDE_DIR Metis_LIBRARY)
