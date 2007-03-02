# - Try to find the gpgme library
# Once done this will define
#
#  GPGME_FOUND - system has the gpgme library
#  GPGME_INCLUDES - the gpgme include directory
#  GPGME_LIBRARIES - The libraries needed to use gpgme

include (MacroEnsureVersion)

# if not already in cache
if (NOT GPGME_LIBRARIES OR NOT GPGME_INCLUDES)
  set(GPGME_FOUND FALSE)

  FIND_PROGRAM(GPGMECONFIG_EXECUTABLE NAMES gpgme-config)

  # if gpgme-config has been found
  if (GPGMECONFIG_EXECUTABLE AND GPGME_VERSION)

    EXEC_PROGRAM(${GPGMECONFIG_EXECUTABLE} ARGS --version OUTPUT_VARIABLE GPGME_VERSION)

    MACRO_ENSURE_VERSION( "0.4.5" ${GPGME_VERSION} GPGME_INSTALLED_VERSION_OK )

    if (GPGME_INSTALLED_VERSION_OK)

      EXEC_PROGRAM(${GPGMECONFIG_EXECUTABLE} ARGS --libs OUTPUT_VARIABLE GPGME_LIBRARIES)

      # append -lgpg-error to the list of libraries, if necessary
      if (NOT GPGME_LIBRARIES MATCHES "lgpg-error")
        set(GPGME_LIBRARIES "${GPGME_LIBRARIES} -lgpg-error")
      endif (NOT GPGME_LIBRARIES MATCHES "lgpg-error")

      EXEC_PROGRAM(${GPGMECONFIG_EXECUTABLE} ARGS --cflags OUTPUT_VARIABLE GPGME_CFLAGS)
      if (GPGME_CFLAGS)
        string(REGEX REPLACE "(\r?\n)+$" "" GPGME_CFLAGS "${GPGME_CFLAGS}")
        string(REGEX REPLACE " *-I" ";" GPGME_INCLUDES "${GPGME_CFLAGS}")
      endif (GPGME_CFLAGS)

      # ensure that they are cached
      set(GPGME_INCLUDES ${GPGME_INCLUDES} CACHE INTERNAL "The gpgme include paths")
      set(GPGME_LIBRARIES ${GPGME_LIBRARIES} CACHE INTERNAL "The gpgme libraries")

    else (GPGME_INSTALLED_VERSION_OK)
      message(STATUS "The installed version of gpgme is too old: ${GPGME_VERSION}")
    endif (GPGME_INSTALLED_VERSION_OK)

  endif (GPGMECONFIG_EXECUTABLE AND GPGME_VERSION)

  if (GPGME_LIBRARIES)
      set(GPGME_FOUND TRUE)
      #message(STATUS "Found gpgme: includes: ${GPGME_INCLUDES} libs: ${GPGME_LIBRARIES}")
      message(STATUS "Found gpgme using ${GPGMECONFIG_EXECUTABLE}.")
  endif (GPGME_LIBRARIES)

  #if (NOT GPGME_FOUND)
  #    message(FATAL_ERROR "You are missing gpgme 0.4.5 or higher.
  #  Download gpgme >= 0.4.5 from ftp://ftp.gnupg.org/gcrypt/alpha/gpgme")
  #endif (NOT GPGME_FOUND)
else (NOT GPGME_LIBRARIES OR NOT GPGME_INCLUDES)
  # It was found before, assume it's OK
  set(GPGME_FOUND TRUE)
endif (NOT GPGME_LIBRARIES OR NOT GPGME_INCLUDES)
