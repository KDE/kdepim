# - Try to find the gpgme library
# Once done this will define
#
#  GPGME_FOUND - system has the gpgme library
#  GPGME_INCLUDE_DIR - the gpgme include directory
#  GPGME_LIBRARIES - The libraries needed to use gpgme

# if not already in cache
IF (NOT GPGME_LIBRARIES OR NOT GPGME_INCLUDE_DIR)
  set(GPGME_FOUND FALSE)

  FIND_PROGRAM(GPGMECONFIG_EXECUTABLE NAMES gpgme-config)

  # if gpgme-config has been found
  IF (GPGMECONFIG_EXECUTABLE)
    EXEC_PROGRAM(${GPGMECONFIG_EXECUTABLE} ARGS --libs RETURN_VALUE _return_VALUE OUTPUT_VARIABLE GPGME_LIBRARIES)

    EXEC_PROGRAM(${GPGMECONFIG_EXECUTABLE} ARGS --cflags RETURN_VALUE _return_VALUE OUTPUT_VARIABLE GPGME_CFLAGS)
    IF (GPGME_CFLAGS)
      # problem: here I get two identical -I directives, and having two paths in INCLUDE_DIR breaks
      # so for now I just keep the first -Ifoo, but this might need to be fixed at some point...
      STRING(REGEX REPLACE "-I([^ ]*).*" "\\1" GPGME_INCLUDE_DIR "${GPGME_CFLAGS}")
    ENDIF (GPGME_CFLAGS)

    # ensure that they are cached
    set(GPGME_INCLUDE_DIR ${GPGME_INCLUDE_DIR} CACHE INTERNAL "The gpgme include path")
    set(GPGME_LIBRARIES ${GPGME_LIBRARIES} CACHE INTERNAL "The gpgme libraries")

  ENDIF (GPGMECONFIG_EXECUTABLE)

  IF (GPGME_LIBRARIES AND GPGME_INCLUDE_DIR)
      set(GPGME_FOUND TRUE)
      #message(STATUS "Found gpgme: includes: ${GPGME_INCLUDE_DIR} libs: ${GPGME_LIBRARIES}")
      message(STATUS "Found gpgme using ${GPGMECONFIG_EXECUTABLE}.")
  ENDIF (GPGME_LIBRARIES AND GPGME_INCLUDE_DIR)

  IF (NOT GPGME_FOUND)
      message(FATAL_ERROR "You are missing gpgme 0.4.5 or higher.
    Download gpgme >= 0.4.5 from ftp://ftp.gnupg.org/gcrypt/alpha/gpgme
    or use the --with-gpgme-prefix=/path/where/gpgme/is/installed option.")
  ENDIF (NOT GPGME_FOUND)
ENDIF (NOT GPGME_LIBRARIES OR NOT GPGME_INCLUDE_DIR)
