# Find libkrss - Akonadi-based RSS handling library
#
# This module defines
#  LIBKRSS_FOUND - whether the libkrss library was found
#  LIBKRSS_LIBRARIES - the libkrss library
#  LIBKRSS_INCLUDE_DIR - the include path of the libkrss library
#

if (LIBKRSS_INCLUDE_DIR AND LIBKRSS_LIBRARIES)

  # Already in cache
  set (LIBKRSS_FOUND TRUE)

else (LIBKRSS_INCLUDE_DIR AND LIBKRSS_LIBRARIES)

  find_library (LIBKRSS_LIBRARIES
    NAMES
    krss
    PATHS
    ${LIB_INSTALL_DIR}
    ${KDE4_LIB_DIR}
  )

  find_path (LIBKRSS_INCLUDE_DIR
    NAMES
    krss_export.h
    PATH_SUFFIXES
    krss
    PATHS
    ${INCLUDE_INSTALL_DIR}
    ${KDE4_INCLUDE_DIR}
  )

  include(FindPackageHandleStandardArgs)
  find_package_handle_standard_args(LibKRSS DEFAULT_MSG LIBKRSS_LIBRARIES LIBKRSS_INCLUDE_DIR)

endif (LIBKRSS_INCLUDE_DIR AND LIBKRSS_LIBRARIES)
