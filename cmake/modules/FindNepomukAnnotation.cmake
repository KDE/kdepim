# Once done this will define
#
# NepomukAnnotation requires Nepomuk, so this module checks for Soprano too.
#
#  NEPOMUK_ANNOTATION_FOUND - system has Nepomuk
#  NEPOMUK_ANNOTATION_INCLUDE_DIR - the Nepomuk include directory
#  NEPOMUK_ANNOTATION_LIBRARIES - Link these to use Nepomuk Annotation
#  NEPOMUK_ANNOTATION_DEFINITIONS - Compiler switches required for using Nepomuk
#

if (NOT DEFINED Nepomuk_FOUND)
  macro_optional_find_package(Nepomuk)
  macro_log_feature(Nepomuk_FOUND "Nepomuk" "Semantic Desktop" "" FALSE "" "Nepomuk is needed for NepomukAnnotation")
endif (NOT DEFINED Nepomuk_FOUND)

if (Nepomuk_FOUND)

  set (NEPOMUK_ANNOTATION_FIND_REQUIRED ${NepomukAnnotation_FIND_REQUIRED})
  if (NEPOMUK_ANNOTATION_INCLUDE_DIR AND NEPOMUK_ANNOTATION_LIBRARIES)

    # Already in cache, be silent
    set(NEPOMUK_ANNOTATION_FIND_QUIETLY TRUE)

  else (NEPOMUK_ANNOTATION_INCLUDE_DIR AND NEPOMUK_ANNOTATION_LIBRARIES)
    find_path(NEPOMUK_ANNOTATION_INCLUDE_DIR
      NAMES
      nepomuk/term.h
      PATHS
      ${KDE4_INCLUDE_DIR}
      ${INCLUDE_INSTALL_DIR}
      )

    find_library(NEPOMUK_ANNOTATION_LIBRARIES
      NAMES
      nepomukannotation
      PATHS
      ${KDE4_LIB_DIR}
      ${LIB_INSTALL_DIR}
      )

    mark_as_advanced(NEPOMUK_ANNOTATION_INCLUDE_DIR NEPOMUK_ANNOTATION_LIBRARIES)

  endif (NEPOMUK_ANNOTATION_INCLUDE_DIR AND NEPOMUK_ANNOTATION_LIBRARIES)

  include(FindPackageHandleStandardArgs)
  find_package_handle_standard_args(NEPOMUK_ANNOTATION DEFAULT_MSG 
                                    NEPOMUK_ANNOTATION_LIBRARIES NEPOMUK_ANNOTATION_INCLUDE_DIR)
  #to retain backward compatibility
  set (NepomukAnnotation_FOUND ${NEPOMUK_ANNOTATION_FOUND})

endif (Nepomuk_FOUND)
