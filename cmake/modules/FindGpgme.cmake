# - Try to find the gpgme library
# Once done this will define
#
#  GPGME_FOUND - system has the gpgme library
#  GPGME_INCLUDES - the gpgme include directory
#  GPGME_LIBRARIES - The libraries needed to use gpgme

# if not already in cache
IF (NOT GPGME_LIBRARIES OR NOT GPGME_INCLUDES)
  set(GPGME_FOUND FALSE)

  FIND_PROGRAM(GPGMECONFIG_EXECUTABLE NAMES gpgme-config)

  # if gpgme-config has been found
  IF (GPGMECONFIG_EXECUTABLE)
    EXEC_PROGRAM(${GPGMECONFIG_EXECUTABLE} ARGS --libs RETURN_VALUE _return_VALUE OUTPUT_VARIABLE GPGME_LIBRARIES)

    # append -lgpg-error to the list of libraries, if necessary
    IF (NOT GPGME_LIBRARIES MATCHES "lgpg-error")
      set(GPGME_LIBRARIES "${GPGME_LIBRARIES} -lgpg-error")
    ENDIF (NOT GPGME_LIBRARIES MATCHES "lgpg-error")

    EXEC_PROGRAM(${GPGMECONFIG_EXECUTABLE} ARGS --cflags RETURN_VALUE _return_VALUE OUTPUT_VARIABLE GPGME_CFLAGS)
    IF (GPGME_CFLAGS)
      string(REGEX REPLACE "(\r?\n)+$" "" GPGME_CFLAGS "${GPGME_CFLAGS}")
      string(REGEX REPLACE " *-I" ";" GPGME_INCLUDES "${GPGME_CFLAGS}")
    ENDIF (GPGME_CFLAGS)

    # ensure that they are cached
    set(GPGME_INCLUDES ${GPGME_INCLUDES} CACHE INTERNAL "The gpgme include paths")
    set(GPGME_LIBRARIES ${GPGME_LIBRARIES} CACHE INTERNAL "The gpgme libraries")

  ENDIF (GPGMECONFIG_EXECUTABLE)

  IF (GPGME_LIBRARIES)
      set(GPGME_FOUND TRUE)
      #message(STATUS "Found gpgme: includes: ${GPGME_INCLUDES} libs: ${GPGME_LIBRARIES}")
      message(STATUS "Found gpgme using ${GPGMECONFIG_EXECUTABLE}.")
  ENDIF (GPGME_LIBRARIES)

  IF (NOT GPGME_FOUND)
      message(FATAL_ERROR "You are missing gpgme 0.4.5 or higher.
    Download gpgme >= 0.4.5 from ftp://ftp.gnupg.org/gcrypt/alpha/gpgme
    or use the --with-gpgme-prefix=/path/where/gpgme/is/installed option.")
  ENDIF (NOT GPGME_FOUND)
ENDIF (NOT GPGME_LIBRARIES OR NOT GPGME_INCLUDES)
